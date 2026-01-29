
`timescale 1ns/1ps

// ------------------------------------------------------------
// Minimal edge detector used by your DUT
// ------------------------------------------------------------
//module posedge_detector (
//    input  logic clk,
//    input  logic rst,
//    input  logic D,
//    output logic Q
//);
//    logic D_d;
//    always_ff @(posedge clk or posedge rst) begin
//        if (rst) begin
//            D_d <= 1'b0;
//            Q   <= 1'b0;
//        end else begin
//            Q   <= (D && !D_d);
//            D_d <= D;
//        end
//    end
//endmodule


// ------------------------------------------------------------
// TB
// ------------------------------------------------------------
module tb_dma_controller;

    // -----------------------------
    // Clock/reset
    // -----------------------------
    logic aclk;
    logic rst_i;

    initial begin
        aclk = 1'b0;
        forever #5 aclk = ~aclk; // 100MHz
    end

    // -----------------------------
    // DUT I/O
    // -----------------------------
    logic [31:0] m_axi_awaddr;
    logic        m_axi_awvalid;
    logic        m_axi_awready;
    logic [3:0]  m_axi_awlen;
    logic [2:0]  m_axi_awsize;
    logic [1:0]  m_axi_awburst;

    logic [63:0] m_axi_wdata;
    logic        m_axi_wvalid;
    logic        m_axi_wready;
    logic        m_axi_bvalid;
    logic        m_axi_bready;
    logic [1:0]  m_axi_bresp;
    logic [7:0]  m_axi_wstrb;
    logic        m_axi_wlast;

    logic        enable_i;
    logic [63:0] data_i;
    logic        finished_o;
    logic        engaged_o;

    // -----------------------------
    // Instantiate DUT
    // -----------------------------
    dma_controller 
    #(
        .DMA_SIZE(32'h000000A0)
    ) dut
    (
        .aclk(aclk),
        .rst_i(rst_i),

        .m_axi_awaddr(m_axi_awaddr),
        .m_axi_awvalid(m_axi_awvalid),
        .m_axi_awready(m_axi_awready),
        .m_axi_awlen(m_axi_awlen),
        .m_axi_awsize(m_axi_awsize),
        .m_axi_awburst(m_axi_awburst),

        .m_axi_wdata(m_axi_wdata),
        .m_axi_wvalid(m_axi_wvalid),
        .m_axi_wready(m_axi_wready),
        .m_axi_bvalid(m_axi_bvalid),
        .m_axi_bready(m_axi_bready),
        .m_axi_bresp(m_axi_bresp),
        .m_axi_wstrb(m_axi_wstrb),
        .m_axi_wlast(m_axi_wlast),

        .enable_i(enable_i),
        .data_i(data_i),
        .finished_o(finished_o)
    );

    // -----------------------------
    // Wave dump ALWAYS ON
    // -----------------------------
    initial begin
        $dumpfile("dumps/dma_tb.vcd");
        $dumpvars(0, tb_dma_controller);
    end

    // -----------------------------
    // Data source: incrementing pattern
    // -----------------------------
    always_ff @(posedge aclk or posedge rst_i) begin
        if (rst_i) data_i <= 64'd0;
        else       data_i <= data_i + 64'd1;
    end

    // -----------------------------
    // Simple AXI sink model knobs
    // -----------------------------
    typedef enum int {MODE_TEST1 = 1, MODE_TEST2 = 2} tb_mode_t;
    tb_mode_t mode;

    int aw_stall_countdown;
    int b_delay_countdown;
    bit b_pending;

    // "Stall pattern" counters
    int cyc;
    always_ff @(posedge aclk or posedge rst_i) begin
        if (rst_i) cyc <= 0;
        else       cyc <= cyc + 1;
    end

    // Dumb sink behavior
    always_ff @(posedge aclk or posedge rst_i) begin
        if (rst_i) begin
            m_axi_awready <= 1'b1;
            m_axi_wready  <= 1'b1;
            m_axi_bvalid  <= 1'b0;
            m_axi_bresp   <= 2'b00; // OKAY
            b_pending     <= 1'b0;
            aw_stall_countdown <= 0;
            b_delay_countdown  <= 0;
        end else begin
            // defaults each cycle
            m_axi_bvalid <= 1'b0;

            // TEST1: no stalls, BVALID one cycle after last beat
            if (mode == MODE_TEST1) begin
                m_axi_awready <= 1'b1;
                m_axi_wready  <= 1'b1;

                if (m_axi_wvalid && m_axi_wready && m_axi_wlast)
                    b_pending <= 1'b1;

                if (b_pending) begin
                    m_axi_bvalid <= 1'b1;
                    b_pending    <= 1'b0;
                end
            end

            // TEST2: stalls + delayed B response
            if (mode == MODE_TEST2) begin
                // AWREADY: stall for a few cycles right after enable pulse hits
                if (aw_stall_countdown > 0) begin
                    m_axi_awready <= 1'b0;
                    aw_stall_countdown <= aw_stall_countdown - 1;
                end else begin
                    m_axi_awready <= 1'b1;
                end

                // WREADY: simple periodic stall (low every 4th cycle)
                // (keeps it deterministic)
                if ((cyc % 4) == 0) m_axi_wready <= 1'b0;
                else                m_axi_wready <= 1'b1;

                // When last beat is accepted, schedule BVALID after delay
                if (m_axi_wvalid && m_axi_wready && m_axi_wlast) begin
                    b_pending <= 1'b1;
                    b_delay_countdown <= 3; // delay 3 cycles
                end

                if (b_pending) begin
                    if (b_delay_countdown > 0) begin
                        b_delay_countdown <= b_delay_countdown - 1;
                    end else begin
                        m_axi_bvalid <= 1'b1;
                        b_pending    <= 1'b0;
                    end
                end
            end
        end
    end

    // -----------------------------
    // Scoreboard / checks
    // -----------------------------
    int  err;
    int  beats_seen;
    bit  saw_aw;
    bit  saw_b;
    logic [31:0] awaddr_first;
    logic [31:0] awaddr_second;

    // For stall stability check (Test2)
    logic [63:0] wdata_hold;
    bit          holding;

    task automatic reset_scoreboard();
        begin
            err         = 0;
            beats_seen  = 0;
            saw_aw      = 0;
            saw_b       = 0;
            awaddr_first  = 32'h0;
            awaddr_second = 32'h0;
            holding     = 0;
            wdata_hold  = 64'h0;
        end
    endtask

    // Monitor transactions
    always_ff @(posedge aclk or posedge rst_i) begin
        if (rst_i) begin
            reset_scoreboard();
        end else begin
            // AW handshake
            if (m_axi_awvalid && m_axi_awready) begin
                if (!saw_aw) begin
                    saw_aw <= 1;
                    awaddr_first <= m_axi_awaddr;
                end else if (awaddr_second == 32'h0) begin
                    // capture second awaddr for increment check if it happens
                    awaddr_second <= m_axi_awaddr;
                end
            end

            // W stall stability check (only meaningful in test2)
            if (mode == MODE_TEST2) begin
                // Start "holding" when WVALID=1 and WREADY=0
                if (m_axi_wvalid && !m_axi_wready) begin
                    if (!holding) begin
                        holding <= 1;
                        wdata_hold <= m_axi_wdata;
                    end else begin
                        // should remain stable while stalled
                        if (m_axi_wdata !== wdata_hold) begin
                            err <= err + 1;
                            // non-fatal
                            $display("[TB] ERROR: WDATA changed during stall! old=%h new=%h time=%0t",
                                     wdata_hold, m_axi_wdata, $time);
                            wdata_hold <= m_axi_wdata; // keep moving so we don't spam
                        end
                    end
                end else begin
                    holding <= 0;
                end
            end

            // W beat handshake
            if (m_axi_wvalid && m_axi_wready) begin
                // check last only at beat 15
                if (m_axi_wlast && (beats_seen != 15)) begin
                    err <= err + 1;
                    $display("[TB] ERROR: WLAST asserted early at beat=%0d time=%0t", beats_seen, $time);
                end
                if (!m_axi_wlast && (beats_seen == 15)) begin
                    err <= err + 1;
                    $display("[TB] ERROR: WLAST not asserted on final beat (15) time=%0t", $time);
                end

                beats_seen <= beats_seen + 1;
            end

            // B handshake
            if (m_axi_bvalid && m_axi_bready) begin
                saw_b <= 1;
            end
        end
    end

    // -----------------------------
    // Tiny "flip switch" enable
    // -----------------------------
    task automatic pulse_enable();
        begin
            enable_i = 1'b1;
            @(posedge aclk);
            enable_i = 1'b0;
        end
    endtask

    // -----------------------------
    // Run test
    // -----------------------------
    task automatic run_test(input tb_mode_t tmode, input string name);
        int timeout;
        bit pass;

        begin
            mode = tmode;
            reset_scoreboard();

            // Setup stall knobs for test2
            if (mode == MODE_TEST2) begin
                aw_stall_countdown = 3; // stall AWREADY 3 cycles at the start
            end else begin
                aw_stall_countdown = 0;
            end

            // Force your internal enable_meta_w (since your DUT expects external drive)
            force dut.enable_meta_w = enable_i;

            // Flip switch
            pulse_enable();

            // Wait until we see 16 beats + a B response (or timeout)
            timeout = 0;
            while (timeout < 50000) begin
                @(posedge aclk);
                timeout++;

                if (beats_seen >= 16 && saw_b) begin
                    break;
                end
            end

            #0;

            // Simple end checks
            if (!saw_aw) begin
                err++;
                $display("[TB] ERROR: never saw AW handshake in %s", name);
            end
            if (beats_seen != 16) begin
                err++;
                $display("[TB] ERROR: expected 16 beats, saw %0d in %s", beats_seen, name);
            end
            if (!saw_b) begin
                err++;
                $display("[TB] ERROR: never saw B handshake in %s", name);
            end

            // If we saw two bursts, check address increment (optional, doesn't hurt)
            if (awaddr_second != 32'h0) begin
                if (awaddr_second != (awaddr_first + 32'd128)) begin
                    err++;
                    $display("[TB] ERROR: AWADDR did not increment by 128. first=0x%08h second=0x%08h",
                             awaddr_first, awaddr_second);
                end
            end

            pass = (err == 0);

            if (pass) $display("=== %s: PASS ===", name);
            else      $display("=== %s: FAIL (errors=%0d) ===", name, err);

            // release force so we don't lock sim if you edit things later
            release dut.enable_meta_w;

            // breathe
            repeat(5) @(posedge aclk);
        end
    endtask

    // -----------------------------
    // Main
    // -----------------------------
    initial begin
        // init
        rst_i    = 1'b1;
        enable_i = 1'b0;
        mode     = MODE_TEST1;

        // reset
        repeat(5) @(posedge aclk);
        rst_i = 1'b0;

        // Run tests
        run_test(MODE_TEST1, "TEST1_NO_STALLS");
        run_test(MODE_TEST2, "TEST2_WITH_STALLS");

        $display("Waveform written: dma_tb.vcd");
        $display("SIM DONE.");
        $finish;
    end

endmodule
