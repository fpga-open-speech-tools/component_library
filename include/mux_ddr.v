//----------------------------------------------------------------------------
// Copyright 2018, iWavesystems Technologies Pvt. Ltd.
// iWave Confidential Proprietary
//----------------------------------------------------------------------------
// Title       : Mux logic for DDR 4GB access for Arria 10 SX
// Design      : Mux Logic
// File        : mux_ddr.v
//----------------------------------------------------------------------------
// Version     : Ver 1.0
// Generated   : 10/08/2018
// Author      : Tushar Sharma
//----------------------------------------------------------------------------
//  Description :
//      * Mux Logic for address change for full 4GB access.
//---------------------------------------------------------------------------- 

module mux_ddr (
//incoming master
input  wire       clock,
input  wire       reset,
input  wire [3:0] arria10_hps_0_h2f_axi_master_awid   ,
input  wire [28:0]arria10_hps_0_h2f_axi_master_awaddr ,
input  wire [3:0] arria10_hps_0_h2f_axi_master_awlen  ,
input  wire [2:0] arria10_hps_0_h2f_axi_master_awsize ,
input  wire [1:0] arria10_hps_0_h2f_axi_master_awburst,
input  wire [1:0] arria10_hps_0_h2f_axi_master_awlock ,
input  wire [3:0] arria10_hps_0_h2f_axi_master_awcache,
input  wire [2:0] arria10_hps_0_h2f_axi_master_awprot ,
input  wire       arria10_hps_0_h2f_axi_master_awvalid,
output wire       arria10_hps_0_h2f_axi_master_awready,
input  wire [4:0] arria10_hps_0_h2f_axi_master_awuser ,
input  wire [3:0] arria10_hps_0_h2f_axi_master_wid    ,
input  wire [31:0]arria10_hps_0_h2f_axi_master_wdata  ,
input  wire [3:0] arria10_hps_0_h2f_axi_master_wstrb  ,
input  wire       arria10_hps_0_h2f_axi_master_wlast  ,
input  wire       arria10_hps_0_h2f_axi_master_wvalid ,
output wire       arria10_hps_0_h2f_axi_master_wready ,
output wire [3:0] arria10_hps_0_h2f_axi_master_bid    ,
output wire [1:0] arria10_hps_0_h2f_axi_master_bresp  ,
output wire       arria10_hps_0_h2f_axi_master_bvalid ,
input  wire       arria10_hps_0_h2f_axi_master_bready ,
input  wire [3:0] arria10_hps_0_h2f_axi_master_arid   ,
input  wire [28:0]arria10_hps_0_h2f_axi_master_araddr ,
input  wire [3:0] arria10_hps_0_h2f_axi_master_arlen  ,
input  wire [2:0] arria10_hps_0_h2f_axi_master_arsize ,
input  wire [1:0] arria10_hps_0_h2f_axi_master_arburst,
input  wire [1:0] arria10_hps_0_h2f_axi_master_arlock ,
input  wire [3:0] arria10_hps_0_h2f_axi_master_arcache,
input  wire [2:0] arria10_hps_0_h2f_axi_master_arprot ,
input  wire       arria10_hps_0_h2f_axi_master_arvalid,
output wire       arria10_hps_0_h2f_axi_master_arready,
input  wire [4:0] arria10_hps_0_h2f_axi_master_aruser ,
output wire [3:0] arria10_hps_0_h2f_axi_master_rid    ,
output wire [31:0]arria10_hps_0_h2f_axi_master_rdata  ,
output wire [1:0] arria10_hps_0_h2f_axi_master_rresp  ,
output wire       arria10_hps_0_h2f_axi_master_rlast  ,
output wire       arria10_hps_0_h2f_axi_master_rvalid ,
input wire        arria10_hps_0_h2f_axi_master_rready ,
// outgoing master
output wire [3:0]  hps_master_awid, 
output wire [31:0] hps_master_awaddr,       
output wire [3:0]  hps_master_awlen,        
output wire [2:0]  hps_master_awsize,       
output wire [1:0]  hps_master_awburst,      
output wire [1:0]  hps_master_awlock,       
output wire [3:0]  hps_master_awcache,      
output wire [2:0]  hps_master_awprot,       
output wire        hps_master_awvalid,      
input  wire        hps_master_awready,      
output wire [4:0]  hps_master_awuser,       
output wire [3:0]  hps_master_wid,          
output wire [31:0] hps_master_wdata,        
output wire [3:0]  hps_master_wstrb,        
output wire        hps_master_wlast,        
output wire        hps_master_wvalid,       
input  wire        hps_master_wready,       
input  wire [3:0]  hps_master_bid,          
input  wire [1:0]  hps_master_bresp,        
input  wire        hps_master_bvalid,       
output wire        hps_master_bready,       
output wire [3:0]  hps_master_arid,         
output wire [31:0] hps_master_araddr,       
output wire [3:0]  hps_master_arlen,        
output wire [2:0]  hps_master_arsize,       
output wire [1:0]  hps_master_arburst,      
output wire [1:0]  hps_master_arlock,       
output wire [3:0]  hps_master_arcache,      
output wire [2:0]  hps_master_arprot,       
output wire        hps_master_arvalid,      
input  wire        hps_master_arready,      
output wire [4:0]  hps_master_aruser,       
input  wire [3:0]  hps_master_rid,          
input  wire [31:0] hps_master_rdata,        
input  wire [1:0]  hps_master_rresp,        
input  wire        hps_master_rlast,        
input  wire        hps_master_rvalid,
output  wire       hps_master_rready,
//address select line
input  wire [2:0]  add_sel          

);
wire [2:0] add_sel_in;

assign add_sel_in = (add_sel == 3'h1)? 3'h001 :
                    (add_sel == 3'h2)? 3'h010 :
                    (add_sel == 3'h3)? 3'h011 :
                    (add_sel == 3'h4)? 3'h100 :
                    (add_sel == 3'h5)? 3'h101 :
                    (add_sel == 3'h6)? 3'h110 :
                    (add_sel == 3'h7)? 3'h111 : 3'h000;
					
assign hps_master_awid    = arria10_hps_0_h2f_axi_master_awid;   
assign hps_master_awaddr  = ({add_sel_in,arria10_hps_0_h2f_axi_master_awaddr[28:0]});						
assign hps_master_awlen   = arria10_hps_0_h2f_axi_master_awlen  ;
assign hps_master_awsize  = arria10_hps_0_h2f_axi_master_awsize ;
assign hps_master_awburst = arria10_hps_0_h2f_axi_master_awburst;
assign hps_master_awlock  = arria10_hps_0_h2f_axi_master_awlock ;
assign hps_master_awcache = arria10_hps_0_h2f_axi_master_awcache;
assign hps_master_awprot  = arria10_hps_0_h2f_axi_master_awprot ;
assign hps_master_awvalid = arria10_hps_0_h2f_axi_master_awvalid;
assign arria10_hps_0_h2f_axi_master_awready = hps_master_awready;
assign hps_master_awuser  = arria10_hps_0_h2f_axi_master_awuser ;
assign hps_master_wid     = arria10_hps_0_h2f_axi_master_wid    ;
assign hps_master_wdata   = arria10_hps_0_h2f_axi_master_wdata  ;
assign hps_master_wstrb   = arria10_hps_0_h2f_axi_master_wstrb  ;
assign hps_master_wlast   = arria10_hps_0_h2f_axi_master_wlast  ;
assign hps_master_wvalid  = arria10_hps_0_h2f_axi_master_wvalid ;
assign arria10_hps_0_h2f_axi_master_wready = hps_master_wready  ;
assign arria10_hps_0_h2f_axi_master_bid    = hps_master_bid     ;  
assign arria10_hps_0_h2f_axi_master_bresp  = hps_master_bresp   ;
assign arria10_hps_0_h2f_axi_master_bvalid = hps_master_bvalid  ;
assign hps_master_bready  = arria10_hps_0_h2f_axi_master_bready ;
assign hps_master_arid    = arria10_hps_0_h2f_axi_master_arid   ; 
assign hps_master_araddr  = ({add_sel_in,arria10_hps_0_h2f_axi_master_araddr[28:0]}); 
assign hps_master_arlen   = arria10_hps_0_h2f_axi_master_arlen  ; 
assign hps_master_arsize  = arria10_hps_0_h2f_axi_master_arsize ;
assign hps_master_arburst = arria10_hps_0_h2f_axi_master_arburst;
assign hps_master_arlock  = arria10_hps_0_h2f_axi_master_arlock ;
assign hps_master_arcache = arria10_hps_0_h2f_axi_master_arcache;
assign hps_master_arprot  = arria10_hps_0_h2f_axi_master_arprot ;
assign hps_master_arvalid = arria10_hps_0_h2f_axi_master_arvalid;
assign arria10_hps_0_h2f_axi_master_arready = hps_master_arready;
assign hps_master_aruser  = arria10_hps_0_h2f_axi_master_aruser ;
assign arria10_hps_0_h2f_axi_master_rid    = hps_master_rid     ; 
assign arria10_hps_0_h2f_axi_master_rdata  = hps_master_rdata   ;
assign arria10_hps_0_h2f_axi_master_rresp  = hps_master_rresp   ;
assign arria10_hps_0_h2f_axi_master_rlast  = hps_master_rlast   ;
assign arria10_hps_0_h2f_axi_master_rvalid = hps_master_rvalid  ;
assign hps_master_rready  = arria10_hps_0_h2f_axi_master_rready ;

endmodule