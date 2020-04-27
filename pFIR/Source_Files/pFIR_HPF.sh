#!/bin/bash
input=/root/pFIR/hpf_coefficients.txt
register0="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register0"
register1="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register1"
register2="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register2"
register3="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register3"
register4="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register4"
register5="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register5"
register6="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register6"
register7="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register7"
register8="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register8"
register9="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register9"
register10="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register10"
register11="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register11"
register12="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register12"
register13="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register13"
register14="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register14"
register15="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register15"
register16="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register16"
register17="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register17"
register18="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register18"
register19="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register19"
register20="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register20"
register21="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register21"
register22="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register22"
register23="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register23"
register24="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register24"
register25="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register25"
register26="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register26"
register27="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register27"
register28="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register28"
register29="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register29"
register30="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register30"
register31="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register31"
register32="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register32"
register33="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register33"
register34="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register34"
register35="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register35"
register36="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register36"
register37="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register37"
register38="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register38"
register39="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register39"
register40="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register40"
register41="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register41"
register42="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register42"
register43="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register43"
register44="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register44"
register45="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register45"
register46="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register46"
register47="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register47"
register48="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register48"
register49="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register49"
register50="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register50"
register51="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register51"
register52="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register52"
register53="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register53"
register54="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register54"
register55="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register55"
register56="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register56"
register57="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register57"
register58="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register58"
register59="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register59"
register60="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register60"
register61="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register61"
register62="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register62"
register63="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register63"
register64="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register64"
register65="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register65"
register66="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register66"
register67="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register67"
register68="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register68"
register69="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register69"
register70="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register70"
register71="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register71"
register72="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register72"
register73="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register73"
register74="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register74"
register75="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register75"
register76="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register76"
register77="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register77"
register78="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register78"
register79="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register79"
register80="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register80"
register81="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register81"
register82="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register82"
register83="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register83"
register84="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register84"
register85="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register85"
register86="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register86"
register87="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register87"
register88="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register88"
register89="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register89"
register90="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register90"
register91="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register91"
register92="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register92"
register93="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register93"
register94="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register94"
register95="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register95"
register96="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register96"
register97="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register97"
register98="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register98"
register99="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register99"
register100="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register100"
register101="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register101"
register102="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register102"
register103="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register103"
register104="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register104"
register105="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register105"
register106="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register106"
register107="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register107"
register108="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register108"
register109="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register109"
register110="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register110"
register111="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register111"
register112="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register112"
register113="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register113"
register114="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register114"
register115="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register115"
register116="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register116"
register117="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register117"
register118="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register118"
register119="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register119"
register120="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register120"
register121="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register121"
register122="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register122"
register123="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register123"
register124="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register124"
register125="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register125"
register126="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register126"
register127="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register127"
register128="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register128"
register129="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register129"
register130="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register130"
register131="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register131"
register132="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register132"
register133="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register133"
register134="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register134"
register135="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register135"
register136="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register136"
register137="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register137"
register138="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register138"
register139="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register139"
register140="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register140"
register141="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register141"
register142="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register142"
register143="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register143"
register144="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register144"
register145="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register145"
register146="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register146"
register147="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register147"
register148="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register148"
register149="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register149"
register150="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register150"
register151="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register151"
register152="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register152"
register153="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register153"
register154="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register154"
register155="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register155"
register156="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register156"
register157="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register157"
register158="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register158"
register159="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register159"
register160="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register160"
register161="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register161"
register162="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register162"
register163="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register163"
register164="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register164"
register165="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register165"
register166="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register166"
register167="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register167"
register168="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register168"
register169="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register169"
register170="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register170"
register171="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register171"
register172="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register172"
register173="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register173"
register174="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register174"
register175="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register175"
register176="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register176"
register177="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register177"
register178="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register178"
register179="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register179"
register180="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register180"
register181="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register181"
register182="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register182"
register183="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register183"
register184="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register184"
register185="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register185"
register186="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register186"
register187="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register187"
register188="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register188"
register189="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register189"
register190="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register190"
register191="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register191"
register192="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register192"
register193="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register193"
register194="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register194"
register195="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register195"
register196="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register196"
register197="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register197"
register198="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register198"
register199="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register199"
register200="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register200"
register201="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register201"
register202="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register202"
register203="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register203"
register204="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register204"
register205="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register205"
register206="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register206"
register207="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register207"
register208="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register208"
register209="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register209"
register210="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register210"
register211="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register211"
register212="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register212"
register213="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register213"
register214="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register214"
register215="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register215"
register216="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register216"
register217="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register217"
register218="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register218"
register219="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register219"
register220="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register220"
register221="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register221"
register222="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register222"
register223="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register223"
register224="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register224"
register225="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register225"
register226="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register226"
register227="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register227"
register228="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register228"
register229="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register229"
register230="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register230"
register231="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register231"
register232="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register232"
register233="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register233"
register234="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register234"
register235="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register235"
register236="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register236"
register237="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register237"
register238="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register238"
register239="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register239"
register240="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register240"
register241="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register241"
register242="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register242"
register243="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register243"
register244="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register244"
register245="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register245"
register246="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register246"
register247="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register247"
register248="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register248"
register249="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register249"
register250="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register250"
register251="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register251"
register252="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register252"
register253="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register253"
register254="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register254"
register255="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register255"
register256="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register256"
register257="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register257"
register258="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register258"
register259="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register259"
register260="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register260"
register261="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register261"
register262="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register262"
register263="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register263"
register264="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register264"
register265="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register265"
register266="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register266"
register267="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register267"
register268="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register268"
register269="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register269"
register270="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register270"
register271="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register271"
register272="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register272"
register273="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register273"
register274="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register274"
register275="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register275"
register276="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register276"
register277="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register277"
register278="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register278"
register279="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register279"
register280="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register280"
register281="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register281"
register282="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register282"
register283="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register283"
register284="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register284"
register285="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register285"
register286="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register286"
register287="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register287"
register288="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register288"
register289="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register289"
register290="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register290"
register291="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register291"
register292="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register292"
register293="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register293"
register294="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register294"
register295="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register295"
register296="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register296"
register297="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register297"
register298="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register298"
register299="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register299"
register300="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register300"
register301="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register301"
register302="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register302"
register303="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register303"
register304="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register304"
register305="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register305"
register306="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register306"
register307="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register307"
register308="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register308"
register309="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register309"
register310="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register310"
register311="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register311"
register312="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register312"
register313="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register313"
register314="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register314"
register315="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register315"
register316="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register316"
register317="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register317"
register318="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register318"
register319="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register319"
register320="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register320"
register321="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register321"
register322="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register322"
register323="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register323"
register324="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register324"
register325="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register325"
register326="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register326"
register327="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register327"
register328="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register328"
register329="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register329"
register330="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register330"
register331="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register331"
register332="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register332"
register333="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register333"
register334="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register334"
register335="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register335"
register336="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register336"
register337="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register337"
register338="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register338"
register339="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register339"
register340="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register340"
register341="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register341"
register342="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register342"
register343="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register343"
register344="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register344"
register345="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register345"
register346="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register346"
register347="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register347"
register348="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register348"
register349="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register349"
register350="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register350"
register351="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register351"
register352="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register352"
register353="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register353"
register354="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register354"
register355="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register355"
register356="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register356"
register357="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register357"
register358="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register358"
register359="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register359"
register360="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register360"
register361="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register361"
register362="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register362"
register363="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register363"
register364="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register364"
register365="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register365"
register366="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register366"
register367="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register367"
register368="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register368"
register369="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register369"
register370="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register370"
register371="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register371"
register372="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register372"
register373="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register373"
register374="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register374"
register375="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register375"
register376="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register376"
register377="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register377"
register378="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register378"
register379="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register379"
register380="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register380"
register381="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register381"
register382="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register382"
register383="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register383"
register384="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register384"
register385="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register385"
register386="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register386"
register387="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register387"
register388="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register388"
register389="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register389"
register390="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register390"
register391="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register391"
register392="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register392"
register393="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register393"
register394="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register394"
register395="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register395"
register396="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register396"
register397="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register397"
register398="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register398"
register399="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register399"
register400="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register400"
register401="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register401"
register402="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register402"
register403="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register403"
register404="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register404"
register405="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register405"
register406="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register406"
register407="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register407"
register408="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register408"
register409="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register409"
register410="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register410"
register411="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register411"
register412="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register412"
register413="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register413"
register414="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register414"
register415="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register415"
register416="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register416"
register417="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register417"
register418="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register418"
register419="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register419"
register420="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register420"
register421="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register421"
register422="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register422"
register423="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register423"
register424="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register424"
register425="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register425"
register426="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register426"
register427="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register427"
register428="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register428"
register429="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register429"
register430="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register430"
register431="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register431"
register432="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register432"
register433="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register433"
register434="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register434"
register435="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register435"
register436="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register436"
register437="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register437"
register438="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register438"
register439="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register439"
register440="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register440"
register441="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register441"
register442="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register442"
register443="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register443"
register444="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register444"
register445="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register445"
register446="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register446"
register447="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register447"
register448="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register448"
register449="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register449"
register450="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register450"
register451="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register451"
register452="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register452"
register453="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register453"
register454="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register454"
register455="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register455"
register456="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register456"
register457="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register457"
register458="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register458"
register459="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register459"
register460="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register460"
register461="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register461"
register462="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register462"
register463="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register463"
register464="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register464"
register465="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register465"
register466="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register466"
register467="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register467"
register468="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register468"
register469="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register469"
register470="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register470"
register471="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register471"
register472="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register472"
register473="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register473"
register474="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register474"
register475="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register475"
register476="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register476"
register477="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register477"
register478="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register478"
register479="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register479"
register480="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register480"
register481="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register481"
register482="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register482"
register483="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register483"
register484="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register484"
register485="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register485"
register486="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register486"
register487="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register487"
register488="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register488"
register489="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register489"
register490="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register490"
register491="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register491"
register492="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register492"
register493="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register493"
register494="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register494"
register495="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register495"
register496="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register496"
register497="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register497"
register498="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register498"
register499="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register499"
register500="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register500"
register501="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register501"
register502="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register502"
register503="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register503"
register504="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register504"
register505="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register505"
register506="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register506"
register507="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register507"
register508="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register508"
register509="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register509"
register510="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register510"
register511="/sys/class/fe_DPRAM_248/fe_DPRAM_248/register511"

echo "Programming the FIR filter..."
mapfile -t a < "$input"
echo "$((16#${a[0]}))",| tee "$register0" > /dev/null;
echo "$((16#${a[1]}))",| tee "$register1" > /dev/null;
echo "$((16#${a[2]}))",| tee "$register2" > /dev/null;
echo "$((16#${a[3]}))",| tee "$register3" > /dev/null;
echo "$((16#${a[4]}))",| tee "$register4" > /dev/null;
echo "$((16#${a[5]}))",| tee "$register5" > /dev/null;
echo "$((16#${a[6]}))",| tee "$register6" > /dev/null;
echo "$((16#${a[7]}))",| tee "$register7" > /dev/null;
echo "$((16#${a[8]}))",| tee "$register8" > /dev/null;
echo "$((16#${a[9]}))",| tee "$register9" > /dev/null;
echo "$((16#${a[10]}))",| tee "$register10" > /dev/null;
echo "$((16#${a[11]}))",| tee "$register11" > /dev/null;
echo "$((16#${a[12]}))",| tee "$register12" > /dev/null;
echo "$((16#${a[13]}))",| tee "$register13" > /dev/null;
echo "$((16#${a[14]}))",| tee "$register14" > /dev/null;
echo "$((16#${a[15]}))",| tee "$register15" > /dev/null;
echo "$((16#${a[16]}))",| tee "$register16" > /dev/null;
echo "$((16#${a[17]}))",| tee "$register17" > /dev/null;
echo "$((16#${a[18]}))",| tee "$register18" > /dev/null;
echo "$((16#${a[19]}))",| tee "$register19" > /dev/null;
echo "$((16#${a[20]}))",| tee "$register20" > /dev/null;
echo "$((16#${a[21]}))",| tee "$register21" > /dev/null;
echo "$((16#${a[22]}))",| tee "$register22" > /dev/null;
echo "$((16#${a[23]}))",| tee "$register23" > /dev/null;
echo "$((16#${a[24]}))",| tee "$register24" > /dev/null;
echo "$((16#${a[25]}))",| tee "$register25" > /dev/null;
echo "$((16#${a[26]}))",| tee "$register26" > /dev/null;
echo "$((16#${a[27]}))",| tee "$register27" > /dev/null;
echo "$((16#${a[28]}))",| tee "$register28" > /dev/null;
echo "$((16#${a[29]}))",| tee "$register29" > /dev/null;
echo "$((16#${a[30]}))",| tee "$register30" > /dev/null;
echo "$((16#${a[31]}))",| tee "$register31" > /dev/null;
echo "$((16#${a[32]}))",| tee "$register32" > /dev/null;
echo "$((16#${a[33]}))",| tee "$register33" > /dev/null;
echo "$((16#${a[34]}))",| tee "$register34" > /dev/null;
echo "$((16#${a[35]}))",| tee "$register35" > /dev/null;
echo "$((16#${a[36]}))",| tee "$register36" > /dev/null;
echo "$((16#${a[37]}))",| tee "$register37" > /dev/null;
echo "$((16#${a[38]}))",| tee "$register38" > /dev/null;
echo "$((16#${a[39]}))",| tee "$register39" > /dev/null;
echo "$((16#${a[40]}))",| tee "$register40" > /dev/null;
echo "$((16#${a[41]}))",| tee "$register41" > /dev/null;
echo "$((16#${a[42]}))",| tee "$register42" > /dev/null;
echo "$((16#${a[43]}))",| tee "$register43" > /dev/null;
echo "$((16#${a[44]}))",| tee "$register44" > /dev/null;
echo "$((16#${a[45]}))",| tee "$register45" > /dev/null;
echo "$((16#${a[46]}))",| tee "$register46" > /dev/null;
echo "$((16#${a[47]}))",| tee "$register47" > /dev/null;
echo "$((16#${a[48]}))",| tee "$register48" > /dev/null;
echo "$((16#${a[49]}))",| tee "$register49" > /dev/null;
echo "$((16#${a[50]}))",| tee "$register50" > /dev/null;
echo "$((16#${a[51]}))",| tee "$register51" > /dev/null;
echo "$((16#${a[52]}))",| tee "$register52" > /dev/null;
echo "$((16#${a[53]}))",| tee "$register53" > /dev/null;
echo "$((16#${a[54]}))",| tee "$register54" > /dev/null;
echo "$((16#${a[55]}))",| tee "$register55" > /dev/null;
echo "$((16#${a[56]}))",| tee "$register56" > /dev/null;
echo "$((16#${a[57]}))",| tee "$register57" > /dev/null;
echo "$((16#${a[58]}))",| tee "$register58" > /dev/null;
echo "$((16#${a[59]}))",| tee "$register59" > /dev/null;
echo "$((16#${a[60]}))",| tee "$register60" > /dev/null;
echo "$((16#${a[61]}))",| tee "$register61" > /dev/null;
echo "$((16#${a[62]}))",| tee "$register62" > /dev/null;
echo "$((16#${a[63]}))",| tee "$register63" > /dev/null;
echo "$((16#${a[64]}))",| tee "$register64" > /dev/null;
echo "$((16#${a[65]}))",| tee "$register65" > /dev/null;
echo "$((16#${a[66]}))",| tee "$register66" > /dev/null;
echo "$((16#${a[67]}))",| tee "$register67" > /dev/null;
echo "$((16#${a[68]}))",| tee "$register68" > /dev/null;
echo "$((16#${a[69]}))",| tee "$register69" > /dev/null;
echo "$((16#${a[70]}))",| tee "$register70" > /dev/null;
echo "$((16#${a[71]}))",| tee "$register71" > /dev/null;
echo "$((16#${a[72]}))",| tee "$register72" > /dev/null;
echo "$((16#${a[73]}))",| tee "$register73" > /dev/null;
echo "$((16#${a[74]}))",| tee "$register74" > /dev/null;
echo "$((16#${a[75]}))",| tee "$register75" > /dev/null;
echo "$((16#${a[76]}))",| tee "$register76" > /dev/null;
echo "$((16#${a[77]}))",| tee "$register77" > /dev/null;
echo "$((16#${a[78]}))",| tee "$register78" > /dev/null;
echo "$((16#${a[79]}))",| tee "$register79" > /dev/null;
echo "$((16#${a[80]}))",| tee "$register80" > /dev/null;
echo "$((16#${a[81]}))",| tee "$register81" > /dev/null;
echo "$((16#${a[82]}))",| tee "$register82" > /dev/null;
echo "$((16#${a[83]}))",| tee "$register83" > /dev/null;
echo "$((16#${a[84]}))",| tee "$register84" > /dev/null;
echo "$((16#${a[85]}))",| tee "$register85" > /dev/null;
echo "$((16#${a[86]}))",| tee "$register86" > /dev/null;
echo "$((16#${a[87]}))",| tee "$register87" > /dev/null;
echo "$((16#${a[88]}))",| tee "$register88" > /dev/null;
echo "$((16#${a[89]}))",| tee "$register89" > /dev/null;
echo "$((16#${a[90]}))",| tee "$register90" > /dev/null;
echo "$((16#${a[91]}))",| tee "$register91" > /dev/null;
echo "$((16#${a[92]}))",| tee "$register92" > /dev/null;
echo "$((16#${a[93]}))",| tee "$register93" > /dev/null;
echo "$((16#${a[94]}))",| tee "$register94" > /dev/null;
echo "$((16#${a[95]}))",| tee "$register95" > /dev/null;
echo "$((16#${a[96]}))",| tee "$register96" > /dev/null;
echo "$((16#${a[97]}))",| tee "$register97" > /dev/null;
echo "$((16#${a[98]}))",| tee "$register98" > /dev/null;
echo "$((16#${a[99]}))",| tee "$register99" > /dev/null;
echo "$((16#${a[100]}))",| tee "$register100" > /dev/null;
echo "$((16#${a[101]}))",| tee "$register101" > /dev/null;
echo "$((16#${a[102]}))",| tee "$register102" > /dev/null;
echo "$((16#${a[103]}))",| tee "$register103" > /dev/null;
echo "$((16#${a[104]}))",| tee "$register104" > /dev/null;
echo "$((16#${a[105]}))",| tee "$register105" > /dev/null;
echo "$((16#${a[106]}))",| tee "$register106" > /dev/null;
echo "$((16#${a[107]}))",| tee "$register107" > /dev/null;
echo "$((16#${a[108]}))",| tee "$register108" > /dev/null;
echo "$((16#${a[109]}))",| tee "$register109" > /dev/null;
echo "$((16#${a[110]}))",| tee "$register110" > /dev/null;
echo "$((16#${a[111]}))",| tee "$register111" > /dev/null;
echo "$((16#${a[112]}))",| tee "$register112" > /dev/null;
echo "$((16#${a[113]}))",| tee "$register113" > /dev/null;
echo "$((16#${a[114]}))",| tee "$register114" > /dev/null;
echo "$((16#${a[115]}))",| tee "$register115" > /dev/null;
echo "$((16#${a[116]}))",| tee "$register116" > /dev/null;
echo "$((16#${a[117]}))",| tee "$register117" > /dev/null;
echo "$((16#${a[118]}))",| tee "$register118" > /dev/null;
echo "$((16#${a[119]}))",| tee "$register119" > /dev/null;
echo "$((16#${a[120]}))",| tee "$register120" > /dev/null;
echo "$((16#${a[121]}))",| tee "$register121" > /dev/null;
echo "$((16#${a[122]}))",| tee "$register122" > /dev/null;
echo "$((16#${a[123]}))",| tee "$register123" > /dev/null;
echo "$((16#${a[124]}))",| tee "$register124" > /dev/null;
echo "$((16#${a[125]}))",| tee "$register125" > /dev/null;
echo "$((16#${a[126]}))",| tee "$register126" > /dev/null;
echo "$((16#${a[127]}))",| tee "$register127" > /dev/null;
echo "$((16#${a[128]}))",| tee "$register128" > /dev/null;
echo "$((16#${a[129]}))",| tee "$register129" > /dev/null;
echo "$((16#${a[130]}))",| tee "$register130" > /dev/null;
echo "$((16#${a[131]}))",| tee "$register131" > /dev/null;
echo "$((16#${a[132]}))",| tee "$register132" > /dev/null;
echo "$((16#${a[133]}))",| tee "$register133" > /dev/null;
echo "$((16#${a[134]}))",| tee "$register134" > /dev/null;
echo "$((16#${a[135]}))",| tee "$register135" > /dev/null;
echo "$((16#${a[136]}))",| tee "$register136" > /dev/null;
echo "$((16#${a[137]}))",| tee "$register137" > /dev/null;
echo "$((16#${a[138]}))",| tee "$register138" > /dev/null;
echo "$((16#${a[139]}))",| tee "$register139" > /dev/null;
echo "$((16#${a[140]}))",| tee "$register140" > /dev/null;
echo "$((16#${a[141]}))",| tee "$register141" > /dev/null;
echo "$((16#${a[142]}))",| tee "$register142" > /dev/null;
echo "$((16#${a[143]}))",| tee "$register143" > /dev/null;
echo "$((16#${a[144]}))",| tee "$register144" > /dev/null;
echo "$((16#${a[145]}))",| tee "$register145" > /dev/null;
echo "$((16#${a[146]}))",| tee "$register146" > /dev/null;
echo "$((16#${a[147]}))",| tee "$register147" > /dev/null;
echo "$((16#${a[148]}))",| tee "$register148" > /dev/null;
echo "$((16#${a[149]}))",| tee "$register149" > /dev/null;
echo "$((16#${a[150]}))",| tee "$register150" > /dev/null;
echo "$((16#${a[151]}))",| tee "$register151" > /dev/null;
echo "$((16#${a[152]}))",| tee "$register152" > /dev/null;
echo "$((16#${a[153]}))",| tee "$register153" > /dev/null;
echo "$((16#${a[154]}))",| tee "$register154" > /dev/null;
echo "$((16#${a[155]}))",| tee "$register155" > /dev/null;
echo "$((16#${a[156]}))",| tee "$register156" > /dev/null;
echo "$((16#${a[157]}))",| tee "$register157" > /dev/null;
echo "$((16#${a[158]}))",| tee "$register158" > /dev/null;
echo "$((16#${a[159]}))",| tee "$register159" > /dev/null;
echo "$((16#${a[160]}))",| tee "$register160" > /dev/null;
echo "$((16#${a[161]}))",| tee "$register161" > /dev/null;
echo "$((16#${a[162]}))",| tee "$register162" > /dev/null;
echo "$((16#${a[163]}))",| tee "$register163" > /dev/null;
echo "$((16#${a[164]}))",| tee "$register164" > /dev/null;
echo "$((16#${a[165]}))",| tee "$register165" > /dev/null;
echo "$((16#${a[166]}))",| tee "$register166" > /dev/null;
echo "$((16#${a[167]}))",| tee "$register167" > /dev/null;
echo "$((16#${a[168]}))",| tee "$register168" > /dev/null;
echo "$((16#${a[169]}))",| tee "$register169" > /dev/null;
echo "$((16#${a[170]}))",| tee "$register170" > /dev/null;
echo "$((16#${a[171]}))",| tee "$register171" > /dev/null;
echo "$((16#${a[172]}))",| tee "$register172" > /dev/null;
echo "$((16#${a[173]}))",| tee "$register173" > /dev/null;
echo "$((16#${a[174]}))",| tee "$register174" > /dev/null;
echo "$((16#${a[175]}))",| tee "$register175" > /dev/null;
echo "$((16#${a[176]}))",| tee "$register176" > /dev/null;
echo "$((16#${a[177]}))",| tee "$register177" > /dev/null;
echo "$((16#${a[178]}))",| tee "$register178" > /dev/null;
echo "$((16#${a[179]}))",| tee "$register179" > /dev/null;
echo "$((16#${a[180]}))",| tee "$register180" > /dev/null;
echo "$((16#${a[181]}))",| tee "$register181" > /dev/null;
echo "$((16#${a[182]}))",| tee "$register182" > /dev/null;
echo "$((16#${a[183]}))",| tee "$register183" > /dev/null;
echo "$((16#${a[184]}))",| tee "$register184" > /dev/null;
echo "$((16#${a[185]}))",| tee "$register185" > /dev/null;
echo "$((16#${a[186]}))",| tee "$register186" > /dev/null;
echo "$((16#${a[187]}))",| tee "$register187" > /dev/null;
echo "$((16#${a[188]}))",| tee "$register188" > /dev/null;
echo "$((16#${a[189]}))",| tee "$register189" > /dev/null;
echo "$((16#${a[190]}))",| tee "$register190" > /dev/null;
echo "$((16#${a[191]}))",| tee "$register191" > /dev/null;
echo "$((16#${a[192]}))",| tee "$register192" > /dev/null;
echo "$((16#${a[193]}))",| tee "$register193" > /dev/null;
echo "$((16#${a[194]}))",| tee "$register194" > /dev/null;
echo "$((16#${a[195]}))",| tee "$register195" > /dev/null;
echo "$((16#${a[196]}))",| tee "$register196" > /dev/null;
echo "$((16#${a[197]}))",| tee "$register197" > /dev/null;
echo "$((16#${a[198]}))",| tee "$register198" > /dev/null;
echo "$((16#${a[199]}))",| tee "$register199" > /dev/null;
echo "$((16#${a[200]}))",| tee "$register200" > /dev/null;
echo "$((16#${a[201]}))",| tee "$register201" > /dev/null;
echo "$((16#${a[202]}))",| tee "$register202" > /dev/null;
echo "$((16#${a[203]}))",| tee "$register203" > /dev/null;
echo "$((16#${a[204]}))",| tee "$register204" > /dev/null;
echo "$((16#${a[205]}))",| tee "$register205" > /dev/null;
echo "$((16#${a[206]}))",| tee "$register206" > /dev/null;
echo "$((16#${a[207]}))",| tee "$register207" > /dev/null;
echo "$((16#${a[208]}))",| tee "$register208" > /dev/null;
echo "$((16#${a[209]}))",| tee "$register209" > /dev/null;
echo "$((16#${a[210]}))",| tee "$register210" > /dev/null;
echo "$((16#${a[211]}))",| tee "$register211" > /dev/null;
echo "$((16#${a[212]}))",| tee "$register212" > /dev/null;
echo "$((16#${a[213]}))",| tee "$register213" > /dev/null;
echo "$((16#${a[214]}))",| tee "$register214" > /dev/null;
echo "$((16#${a[215]}))",| tee "$register215" > /dev/null;
echo "$((16#${a[216]}))",| tee "$register216" > /dev/null;
echo "$((16#${a[217]}))",| tee "$register217" > /dev/null;
echo "$((16#${a[218]}))",| tee "$register218" > /dev/null;
echo "$((16#${a[219]}))",| tee "$register219" > /dev/null;
echo "$((16#${a[220]}))",| tee "$register220" > /dev/null;
echo "$((16#${a[221]}))",| tee "$register221" > /dev/null;
echo "$((16#${a[222]}))",| tee "$register222" > /dev/null;
echo "$((16#${a[223]}))",| tee "$register223" > /dev/null;
echo "$((16#${a[224]}))",| tee "$register224" > /dev/null;
echo "$((16#${a[225]}))",| tee "$register225" > /dev/null;
echo "$((16#${a[226]}))",| tee "$register226" > /dev/null;
echo "$((16#${a[227]}))",| tee "$register227" > /dev/null;
echo "$((16#${a[228]}))",| tee "$register228" > /dev/null;
echo "$((16#${a[229]}))",| tee "$register229" > /dev/null;
echo "$((16#${a[230]}))",| tee "$register230" > /dev/null;
echo "$((16#${a[231]}))",| tee "$register231" > /dev/null;
echo "$((16#${a[232]}))",| tee "$register232" > /dev/null;
echo "$((16#${a[233]}))",| tee "$register233" > /dev/null;
echo "$((16#${a[234]}))",| tee "$register234" > /dev/null;
echo "$((16#${a[235]}))",| tee "$register235" > /dev/null;
echo "$((16#${a[236]}))",| tee "$register236" > /dev/null;
echo "$((16#${a[237]}))",| tee "$register237" > /dev/null;
echo "$((16#${a[238]}))",| tee "$register238" > /dev/null;
echo "$((16#${a[239]}))",| tee "$register239" > /dev/null;
echo "$((16#${a[240]}))",| tee "$register240" > /dev/null;
echo "$((16#${a[241]}))",| tee "$register241" > /dev/null;
echo "$((16#${a[242]}))",| tee "$register242" > /dev/null;
echo "$((16#${a[243]}))",| tee "$register243" > /dev/null;
echo "$((16#${a[244]}))",| tee "$register244" > /dev/null;
echo "$((16#${a[245]}))",| tee "$register245" > /dev/null;
echo "$((16#${a[246]}))",| tee "$register246" > /dev/null;
echo "$((16#${a[247]}))",| tee "$register247" > /dev/null;
echo "$((16#${a[248]}))",| tee "$register248" > /dev/null;
echo "$((16#${a[249]}))",| tee "$register249" > /dev/null;
echo "$((16#${a[250]}))",| tee "$register250" > /dev/null;
echo "$((16#${a[251]}))",| tee "$register251" > /dev/null;
echo "$((16#${a[252]}))",| tee "$register252" > /dev/null;
echo "$((16#${a[253]}))",| tee "$register253" > /dev/null;
echo "$((16#${a[254]}))",| tee "$register254" > /dev/null;
echo "$((16#${a[255]}))",| tee "$register255" > /dev/null;
echo "$((16#${a[256]}))",| tee "$register256" > /dev/null;
echo "$((16#${a[257]}))",| tee "$register257" > /dev/null;
echo "$((16#${a[258]}))",| tee "$register258" > /dev/null;
echo "$((16#${a[259]}))",| tee "$register259" > /dev/null;
echo "$((16#${a[260]}))",| tee "$register260" > /dev/null;
echo "$((16#${a[261]}))",| tee "$register261" > /dev/null;
echo "$((16#${a[262]}))",| tee "$register262" > /dev/null;
echo "$((16#${a[263]}))",| tee "$register263" > /dev/null;
echo "$((16#${a[264]}))",| tee "$register264" > /dev/null;
echo "$((16#${a[265]}))",| tee "$register265" > /dev/null;
echo "$((16#${a[266]}))",| tee "$register266" > /dev/null;
echo "$((16#${a[267]}))",| tee "$register267" > /dev/null;
echo "$((16#${a[268]}))",| tee "$register268" > /dev/null;
echo "$((16#${a[269]}))",| tee "$register269" > /dev/null;
echo "$((16#${a[270]}))",| tee "$register270" > /dev/null;
echo "$((16#${a[271]}))",| tee "$register271" > /dev/null;
echo "$((16#${a[272]}))",| tee "$register272" > /dev/null;
echo "$((16#${a[273]}))",| tee "$register273" > /dev/null;
echo "$((16#${a[274]}))",| tee "$register274" > /dev/null;
echo "$((16#${a[275]}))",| tee "$register275" > /dev/null;
echo "$((16#${a[276]}))",| tee "$register276" > /dev/null;
echo "$((16#${a[277]}))",| tee "$register277" > /dev/null;
echo "$((16#${a[278]}))",| tee "$register278" > /dev/null;
echo "$((16#${a[279]}))",| tee "$register279" > /dev/null;
echo "$((16#${a[280]}))",| tee "$register280" > /dev/null;
echo "$((16#${a[281]}))",| tee "$register281" > /dev/null;
echo "$((16#${a[282]}))",| tee "$register282" > /dev/null;
echo "$((16#${a[283]}))",| tee "$register283" > /dev/null;
echo "$((16#${a[284]}))",| tee "$register284" > /dev/null;
echo "$((16#${a[285]}))",| tee "$register285" > /dev/null;
echo "$((16#${a[286]}))",| tee "$register286" > /dev/null;
echo "$((16#${a[287]}))",| tee "$register287" > /dev/null;
echo "$((16#${a[288]}))",| tee "$register288" > /dev/null;
echo "$((16#${a[289]}))",| tee "$register289" > /dev/null;
echo "$((16#${a[290]}))",| tee "$register290" > /dev/null;
echo "$((16#${a[291]}))",| tee "$register291" > /dev/null;
echo "$((16#${a[292]}))",| tee "$register292" > /dev/null;
echo "$((16#${a[293]}))",| tee "$register293" > /dev/null;
echo "$((16#${a[294]}))",| tee "$register294" > /dev/null;
echo "$((16#${a[295]}))",| tee "$register295" > /dev/null;
echo "$((16#${a[296]}))",| tee "$register296" > /dev/null;
echo "$((16#${a[297]}))",| tee "$register297" > /dev/null;
echo "$((16#${a[298]}))",| tee "$register298" > /dev/null;
echo "$((16#${a[299]}))",| tee "$register299" > /dev/null;
echo "$((16#${a[300]}))",| tee "$register300" > /dev/null;
echo "$((16#${a[301]}))",| tee "$register301" > /dev/null;
echo "$((16#${a[302]}))",| tee "$register302" > /dev/null;
echo "$((16#${a[303]}))",| tee "$register303" > /dev/null;
echo "$((16#${a[304]}))",| tee "$register304" > /dev/null;
echo "$((16#${a[305]}))",| tee "$register305" > /dev/null;
echo "$((16#${a[306]}))",| tee "$register306" > /dev/null;
echo "$((16#${a[307]}))",| tee "$register307" > /dev/null;
echo "$((16#${a[308]}))",| tee "$register308" > /dev/null;
echo "$((16#${a[309]}))",| tee "$register309" > /dev/null;
echo "$((16#${a[310]}))",| tee "$register310" > /dev/null;
echo "$((16#${a[311]}))",| tee "$register311" > /dev/null;
echo "$((16#${a[312]}))",| tee "$register312" > /dev/null;
echo "$((16#${a[313]}))",| tee "$register313" > /dev/null;
echo "$((16#${a[314]}))",| tee "$register314" > /dev/null;
echo "$((16#${a[315]}))",| tee "$register315" > /dev/null;
echo "$((16#${a[316]}))",| tee "$register316" > /dev/null;
echo "$((16#${a[317]}))",| tee "$register317" > /dev/null;
echo "$((16#${a[318]}))",| tee "$register318" > /dev/null;
echo "$((16#${a[319]}))",| tee "$register319" > /dev/null;
echo "$((16#${a[320]}))",| tee "$register320" > /dev/null;
echo "$((16#${a[321]}))",| tee "$register321" > /dev/null;
echo "$((16#${a[322]}))",| tee "$register322" > /dev/null;
echo "$((16#${a[323]}))",| tee "$register323" > /dev/null;
echo "$((16#${a[324]}))",| tee "$register324" > /dev/null;
echo "$((16#${a[325]}))",| tee "$register325" > /dev/null;
echo "$((16#${a[326]}))",| tee "$register326" > /dev/null;
echo "$((16#${a[327]}))",| tee "$register327" > /dev/null;
echo "$((16#${a[328]}))",| tee "$register328" > /dev/null;
echo "$((16#${a[329]}))",| tee "$register329" > /dev/null;
echo "$((16#${a[330]}))",| tee "$register330" > /dev/null;
echo "$((16#${a[331]}))",| tee "$register331" > /dev/null;
echo "$((16#${a[332]}))",| tee "$register332" > /dev/null;
echo "$((16#${a[333]}))",| tee "$register333" > /dev/null;
echo "$((16#${a[334]}))",| tee "$register334" > /dev/null;
echo "$((16#${a[335]}))",| tee "$register335" > /dev/null;
echo "$((16#${a[336]}))",| tee "$register336" > /dev/null;
echo "$((16#${a[337]}))",| tee "$register337" > /dev/null;
echo "$((16#${a[338]}))",| tee "$register338" > /dev/null;
echo "$((16#${a[339]}))",| tee "$register339" > /dev/null;
echo "$((16#${a[340]}))",| tee "$register340" > /dev/null;
echo "$((16#${a[341]}))",| tee "$register341" > /dev/null;
echo "$((16#${a[342]}))",| tee "$register342" > /dev/null;
echo "$((16#${a[343]}))",| tee "$register343" > /dev/null;
echo "$((16#${a[344]}))",| tee "$register344" > /dev/null;
echo "$((16#${a[345]}))",| tee "$register345" > /dev/null;
echo "$((16#${a[346]}))",| tee "$register346" > /dev/null;
echo "$((16#${a[347]}))",| tee "$register347" > /dev/null;
echo "$((16#${a[348]}))",| tee "$register348" > /dev/null;
echo "$((16#${a[349]}))",| tee "$register349" > /dev/null;
echo "$((16#${a[350]}))",| tee "$register350" > /dev/null;
echo "$((16#${a[351]}))",| tee "$register351" > /dev/null;
echo "$((16#${a[352]}))",| tee "$register352" > /dev/null;
echo "$((16#${a[353]}))",| tee "$register353" > /dev/null;
echo "$((16#${a[354]}))",| tee "$register354" > /dev/null;
echo "$((16#${a[355]}))",| tee "$register355" > /dev/null;
echo "$((16#${a[356]}))",| tee "$register356" > /dev/null;
echo "$((16#${a[357]}))",| tee "$register357" > /dev/null;
echo "$((16#${a[358]}))",| tee "$register358" > /dev/null;
echo "$((16#${a[359]}))",| tee "$register359" > /dev/null;
echo "$((16#${a[360]}))",| tee "$register360" > /dev/null;
echo "$((16#${a[361]}))",| tee "$register361" > /dev/null;
echo "$((16#${a[362]}))",| tee "$register362" > /dev/null;
echo "$((16#${a[363]}))",| tee "$register363" > /dev/null;
echo "$((16#${a[364]}))",| tee "$register364" > /dev/null;
echo "$((16#${a[365]}))",| tee "$register365" > /dev/null;
echo "$((16#${a[366]}))",| tee "$register366" > /dev/null;
echo "$((16#${a[367]}))",| tee "$register367" > /dev/null;
echo "$((16#${a[368]}))",| tee "$register368" > /dev/null;
echo "$((16#${a[369]}))",| tee "$register369" > /dev/null;
echo "$((16#${a[370]}))",| tee "$register370" > /dev/null;
echo "$((16#${a[371]}))",| tee "$register371" > /dev/null;
echo "$((16#${a[372]}))",| tee "$register372" > /dev/null;
echo "$((16#${a[373]}))",| tee "$register373" > /dev/null;
echo "$((16#${a[374]}))",| tee "$register374" > /dev/null;
echo "$((16#${a[375]}))",| tee "$register375" > /dev/null;
echo "$((16#${a[376]}))",| tee "$register376" > /dev/null;
echo "$((16#${a[377]}))",| tee "$register377" > /dev/null;
echo "$((16#${a[378]}))",| tee "$register378" > /dev/null;
echo "$((16#${a[379]}))",| tee "$register379" > /dev/null;
echo "$((16#${a[380]}))",| tee "$register380" > /dev/null;
echo "$((16#${a[381]}))",| tee "$register381" > /dev/null;
echo "$((16#${a[382]}))",| tee "$register382" > /dev/null;
echo "$((16#${a[383]}))",| tee "$register383" > /dev/null;
echo "$((16#${a[384]}))",| tee "$register384" > /dev/null;
echo "$((16#${a[385]}))",| tee "$register385" > /dev/null;
echo "$((16#${a[386]}))",| tee "$register386" > /dev/null;
echo "$((16#${a[387]}))",| tee "$register387" > /dev/null;
echo "$((16#${a[388]}))",| tee "$register388" > /dev/null;
echo "$((16#${a[389]}))",| tee "$register389" > /dev/null;
echo "$((16#${a[390]}))",| tee "$register390" > /dev/null;
echo "$((16#${a[391]}))",| tee "$register391" > /dev/null;
echo "$((16#${a[392]}))",| tee "$register392" > /dev/null;
echo "$((16#${a[393]}))",| tee "$register393" > /dev/null;
echo "$((16#${a[394]}))",| tee "$register394" > /dev/null;
echo "$((16#${a[395]}))",| tee "$register395" > /dev/null;
echo "$((16#${a[396]}))",| tee "$register396" > /dev/null;
echo "$((16#${a[397]}))",| tee "$register397" > /dev/null;
echo "$((16#${a[398]}))",| tee "$register398" > /dev/null;
echo "$((16#${a[399]}))",| tee "$register399" > /dev/null;
echo "$((16#${a[400]}))",| tee "$register400" > /dev/null;
echo "$((16#${a[401]}))",| tee "$register401" > /dev/null;
echo "$((16#${a[402]}))",| tee "$register402" > /dev/null;
echo "$((16#${a[403]}))",| tee "$register403" > /dev/null;
echo "$((16#${a[404]}))",| tee "$register404" > /dev/null;
echo "$((16#${a[405]}))",| tee "$register405" > /dev/null;
echo "$((16#${a[406]}))",| tee "$register406" > /dev/null;
echo "$((16#${a[407]}))",| tee "$register407" > /dev/null;
echo "$((16#${a[408]}))",| tee "$register408" > /dev/null;
echo "$((16#${a[409]}))",| tee "$register409" > /dev/null;
echo "$((16#${a[410]}))",| tee "$register410" > /dev/null;
echo "$((16#${a[411]}))",| tee "$register411" > /dev/null;
echo "$((16#${a[412]}))",| tee "$register412" > /dev/null;
echo "$((16#${a[413]}))",| tee "$register413" > /dev/null;
echo "$((16#${a[414]}))",| tee "$register414" > /dev/null;
echo "$((16#${a[415]}))",| tee "$register415" > /dev/null;
echo "$((16#${a[416]}))",| tee "$register416" > /dev/null;
echo "$((16#${a[417]}))",| tee "$register417" > /dev/null;
echo "$((16#${a[418]}))",| tee "$register418" > /dev/null;
echo "$((16#${a[419]}))",| tee "$register419" > /dev/null;
echo "$((16#${a[420]}))",| tee "$register420" > /dev/null;
echo "$((16#${a[421]}))",| tee "$register421" > /dev/null;
echo "$((16#${a[422]}))",| tee "$register422" > /dev/null;
echo "$((16#${a[423]}))",| tee "$register423" > /dev/null;
echo "$((16#${a[424]}))",| tee "$register424" > /dev/null;
echo "$((16#${a[425]}))",| tee "$register425" > /dev/null;
echo "$((16#${a[426]}))",| tee "$register426" > /dev/null;
echo "$((16#${a[427]}))",| tee "$register427" > /dev/null;
echo "$((16#${a[428]}))",| tee "$register428" > /dev/null;
echo "$((16#${a[429]}))",| tee "$register429" > /dev/null;
echo "$((16#${a[430]}))",| tee "$register430" > /dev/null;
echo "$((16#${a[431]}))",| tee "$register431" > /dev/null;
echo "$((16#${a[432]}))",| tee "$register432" > /dev/null;
echo "$((16#${a[433]}))",| tee "$register433" > /dev/null;
echo "$((16#${a[434]}))",| tee "$register434" > /dev/null;
echo "$((16#${a[435]}))",| tee "$register435" > /dev/null;
echo "$((16#${a[436]}))",| tee "$register436" > /dev/null;
echo "$((16#${a[437]}))",| tee "$register437" > /dev/null;
echo "$((16#${a[438]}))",| tee "$register438" > /dev/null;
echo "$((16#${a[439]}))",| tee "$register439" > /dev/null;
echo "$((16#${a[440]}))",| tee "$register440" > /dev/null;
echo "$((16#${a[441]}))",| tee "$register441" > /dev/null;
echo "$((16#${a[442]}))",| tee "$register442" > /dev/null;
echo "$((16#${a[443]}))",| tee "$register443" > /dev/null;
echo "$((16#${a[444]}))",| tee "$register444" > /dev/null;
echo "$((16#${a[445]}))",| tee "$register445" > /dev/null;
echo "$((16#${a[446]}))",| tee "$register446" > /dev/null;
echo "$((16#${a[447]}))",| tee "$register447" > /dev/null;
echo "$((16#${a[448]}))",| tee "$register448" > /dev/null;
echo "$((16#${a[449]}))",| tee "$register449" > /dev/null;
echo "$((16#${a[450]}))",| tee "$register450" > /dev/null;
echo "$((16#${a[451]}))",| tee "$register451" > /dev/null;
echo "$((16#${a[452]}))",| tee "$register452" > /dev/null;
echo "$((16#${a[453]}))",| tee "$register453" > /dev/null;
echo "$((16#${a[454]}))",| tee "$register454" > /dev/null;
echo "$((16#${a[455]}))",| tee "$register455" > /dev/null;
echo "$((16#${a[456]}))",| tee "$register456" > /dev/null;
echo "$((16#${a[457]}))",| tee "$register457" > /dev/null;
echo "$((16#${a[458]}))",| tee "$register458" > /dev/null;
echo "$((16#${a[459]}))",| tee "$register459" > /dev/null;
echo "$((16#${a[460]}))",| tee "$register460" > /dev/null;
echo "$((16#${a[461]}))",| tee "$register461" > /dev/null;
echo "$((16#${a[462]}))",| tee "$register462" > /dev/null;
echo "$((16#${a[463]}))",| tee "$register463" > /dev/null;
echo "$((16#${a[464]}))",| tee "$register464" > /dev/null;
echo "$((16#${a[465]}))",| tee "$register465" > /dev/null;
echo "$((16#${a[466]}))",| tee "$register466" > /dev/null;
echo "$((16#${a[467]}))",| tee "$register467" > /dev/null;
echo "$((16#${a[468]}))",| tee "$register468" > /dev/null;
echo "$((16#${a[469]}))",| tee "$register469" > /dev/null;
echo "$((16#${a[470]}))",| tee "$register470" > /dev/null;
echo "$((16#${a[471]}))",| tee "$register471" > /dev/null;
echo "$((16#${a[472]}))",| tee "$register472" > /dev/null;
echo "$((16#${a[473]}))",| tee "$register473" > /dev/null;
echo "$((16#${a[474]}))",| tee "$register474" > /dev/null;
echo "$((16#${a[475]}))",| tee "$register475" > /dev/null;
echo "$((16#${a[476]}))",| tee "$register476" > /dev/null;
echo "$((16#${a[477]}))",| tee "$register477" > /dev/null;
echo "$((16#${a[478]}))",| tee "$register478" > /dev/null;
echo "$((16#${a[479]}))",| tee "$register479" > /dev/null;
echo "$((16#${a[480]}))",| tee "$register480" > /dev/null;
echo "$((16#${a[481]}))",| tee "$register481" > /dev/null;
echo "$((16#${a[482]}))",| tee "$register482" > /dev/null;
echo "$((16#${a[483]}))",| tee "$register483" > /dev/null;
echo "$((16#${a[484]}))",| tee "$register484" > /dev/null;
echo "$((16#${a[485]}))",| tee "$register485" > /dev/null;
echo "$((16#${a[486]}))",| tee "$register486" > /dev/null;
echo "$((16#${a[487]}))",| tee "$register487" > /dev/null;
echo "$((16#${a[488]}))",| tee "$register488" > /dev/null;
echo "$((16#${a[489]}))",| tee "$register489" > /dev/null;
echo "$((16#${a[490]}))",| tee "$register490" > /dev/null;
echo "$((16#${a[491]}))",| tee "$register491" > /dev/null;
echo "$((16#${a[492]}))",| tee "$register492" > /dev/null;
echo "$((16#${a[493]}))",| tee "$register493" > /dev/null;
echo "$((16#${a[494]}))",| tee "$register494" > /dev/null;
echo "$((16#${a[495]}))",| tee "$register495" > /dev/null;
echo "$((16#${a[496]}))",| tee "$register496" > /dev/null;
echo "$((16#${a[497]}))",| tee "$register497" > /dev/null;
echo "$((16#${a[498]}))",| tee "$register498" > /dev/null;
echo "$((16#${a[499]}))",| tee "$register499" > /dev/null;
echo "$((16#${a[500]}))",| tee "$register500" > /dev/null;
echo "$((16#${a[501]}))",| tee "$register501" > /dev/null;
echo "$((16#${a[502]}))",| tee "$register502" > /dev/null;
echo "$((16#${a[503]}))",| tee "$register503" > /dev/null;
echo "$((16#${a[504]}))",| tee "$register504" > /dev/null;
echo "$((16#${a[505]}))",| tee "$register505" > /dev/null;
echo "$((16#${a[506]}))",| tee "$register506" > /dev/null;
echo "$((16#${a[507]}))",| tee "$register507" > /dev/null;
echo "$((16#${a[508]}))",| tee "$register508" > /dev/null;
echo "$((16#${a[509]}))",| tee "$register509" > /dev/null;
echo "$((16#${a[510]}))",| tee "$register510" > /dev/null;
echo "$((16#${a[511]}))",| tee "$register511" > /dev/null;

echo "FIR filter successfully programmed!"
