module i2s_core_wrapper (
/* clock control */
	input				clk_ckctrl, // Interface clock
	input				reset_n_ckctrl, // asynchronous, active low
	// APB
	input		[4:0]	paddr_ckctrl, // apb address
	input				penable_ckctrl, // apb enable
	input				pwrite_ckctrl,	// apb write strobe
	input 	[31:0]pwdata_ckctrl, // apb data in
	input				psel_ckctrl, // apb select
	output 	[31:0]prdata_ckctrl, // apb data out
	output			pready_ckctrl, // apb ready
	// Clock inputs, synthesized in PLL or external TCXOs
	input				clk_48_ckctrl, // this clock, divided by mclk_devisor, should be 22.
	input				clk_44_ckctrl,
	// In slave mode, an external master makes the clocks
	input				ext_bclk_ckctrl,
	input				ext_playback_lrclk_ckctrl,
	input				ext_capture_lrclk_ckctrl,
	output			master_slave_mode_ckctrl, // 1 = master, 0 (default) = slave
	// Clock derived outputs
	output			clk_sel_48_44_ckctrl, // 1 = mclk derived from 44, 0 (default) mclk derived from 48
	output			mclk_ckctrl,
	output			bclk_ckctrl,
	output			playback_lrclk_ckctrl,
	output			capture_lrclk_ckctrl,
	

/* output control */
	input 			clk_opt, // Interface clock
	input 			reset_n_opt, // asynchronous, active low
	// APB
	input 	[4:0]	paddr_opt, // apb address
	input 			penable_opt, // apb enable
	input 			pwrite_opt,	// apb write strobe
	input  	[31:0]pwdata_opt, // apb data in
	input 			psel_opt, // apb select
	output 	[31:0]prdata_opt, // apb data out
	output 			pready_opt, // apb ready
	// FIFO interface to playback shift register
	output 	[63:0]playback_fifo_data_opt,
	input 			playback_fifo_read_opt,
	output 			playback_fifo_empty_opt,
	output 			playback_fifo_full_opt,
	input 			playback_fifo_clk_opt,
	// DMA interface, SOCFPGA
	output 			playback_dma_req_opt,
	input 			playback_dma_ack_opt,
	output 			playback_dma_enable_opt,
	// FIFO interface to capture shift register
	input 	[63:0]	capture_fifo_data_opt,
	input 			capture_fifo_write_opt,
	output 			capture_fifo_empty_opt,
	output 			capture_fifo_full_opt,
	input 			capture_fifo_clk_opt,
	// DMA interface, SOCFPGA
	output 			capture_dma_req_opt,
	input 			capture_dma_ack_opt,
	output 			capture_dma_enable_opt

);

	i2s_clkctrl_apb i2s_clkctrl_apb_inst (
		.clk                (clk_ckctrl),                                               //     clock.clk
		.reset_n            (reset_n_ckctrl),                   //     reset.reset_n
		.paddr              (paddr_ckctrl),   // apb_slave.paddr
		.penable            (penable_ckctrl), //          .penable
		.pwrite             (pwrite_ckctrl),  //          .pwrite
		.psel               (psel_ckctrl),    //          .psel
		.prdata             (prdata_ckctrl),  //          .prdata
		.pready             (pready_ckctrl),  //          .pready
		.pwdata             (pwdata_ckctrl),  //          .pwdata
		.clk_48             (clk_48_ckctrl),                                       //    clk_48.clk
		.clk_44             (clk_44_ckctrl),                                       //    clk_44.clk
		.playback_lrclk     (playback_lrclk_ckctrl),              //   conduit.playback_lrclk
		.clk_sel_48_44      (clk_sel_48_44_ckctrl),               //          .clk_sel_48_44
		.master_slave_mode  (master_slave_mode_ckctrl),           //          .master_slave_mode
		.bclk               (bclk_ckctrl),                        //          .bclk
		.capture_lrclk      (capture_lrclk_ckctrl),               //          .capture_lrclk
		.mclk               (mclk_ckctrl),                            //      mclk.clk
		.ext_bclk           (ext_bclk_ckctrl),                            //       ext.bclk
		.ext_capture_lrclk  (ext_capture_lrclk_ckctrl),                   //          .capture_lrclk
		.ext_playback_lrclk (ext_playback_lrclk_ckctrl)                   //          .playback_lrclk
	);

	i2s_output_apb i2s_output_apb_inst (
		.reset_n             (reset_n_opt),                  //         reset.reset_n
		.paddr               (paddr_opt),   //     apb_slave.paddr
		.penable             (penable_opt), //              .penable
		.pwrite              (pwrite_opt),  //              .pwrite
		.pwdata              (pwdata_opt),  //              .pwdata
		.psel                (psel_opt),    //              .psel
		.prdata              (prdata_opt),  //              .prdata
		.pready              (pready_opt),  //              .pready
		.clk                 (clk_opt),                                              //           clk.clk
		.playback_fifo_read  (playback_fifo_read_opt),                  // playback_fifo.read
		.playback_fifo_empty (playback_fifo_empty_opt),                 //              .empty
		.playback_fifo_full  (playback_fifo_full_opt),                  //              .full
		.playback_fifo_clk   (playback_fifo_clk_opt),                   //              .clk
		.playback_fifo_data  (playback_fifo_data_opt),                  //              .data
		.playback_dma_req    (playback_dma_req_opt),                    //  playback_dma.req
		.playback_dma_ack    (playback_dma_ack_opt),                    //              .ack
		.playback_dma_enable (playback_dma_enable_opt),                 //              .enable
		.capture_fifo_data   (capture_fifo_data_opt),                   //  capture_fifo.data
		.capture_fifo_write  (capture_fifo_write_opt),                  //              .write
		.capture_fifo_full   (capture_fifo_full_opt),                   //              .full
		.capture_fifo_clk    (capture_fifo_clk_opt),                    //              .clk
		.capture_fifo_empty  (capture_fifo_empty_opt),                  //              .empty
		.capture_dma_req     (capture_dma_req_opt),                     //   capture_dma.req
		.capture_dma_ack     (capture_dma_ack_opt),                     //              .ack
		.capture_dma_enable  (capture_dma_enable_opt)                   //              .enable
	);


endmodule
