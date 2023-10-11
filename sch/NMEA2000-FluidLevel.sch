<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.6.2">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="24" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="88" name="SimResults" color="9" fill="1" visible="yes" active="yes"/>
<layer number="89" name="SimProbes" color="9" fill="1" visible="yes" active="yes"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
<layer number="99" name="SpiceOrder" color="5" fill="1" visible="yes" active="yes"/>
<layer number="250" name="Descript" color="3" fill="1" visible="no" active="yes"/>
<layer number="254" name="OrgLBR" color="13" fill="1" visible="no" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="_esp32">
<packages>
<package name="ESP32_NODEMCU">
<pad name="13" x="0" y="2.54" drill="0.6"/>
<pad name="12" x="0" y="5.08" drill="0.6"/>
<pad name="11" x="0" y="7.62" drill="0.6"/>
<pad name="10" x="0" y="10.16" drill="0.6"/>
<pad name="9" x="0" y="12.7" drill="0.6"/>
<pad name="8" x="0" y="15.24" drill="0.6"/>
<pad name="7" x="0" y="17.78" drill="0.6"/>
<pad name="6" x="0" y="20.32" drill="0.6"/>
<pad name="5" x="0" y="22.86" drill="0.6"/>
<pad name="4" x="0" y="25.4" drill="0.6"/>
<pad name="3" x="0" y="27.94" drill="0.6"/>
<pad name="2" x="0" y="30.48" drill="0.6"/>
<pad name="1" x="0" y="33.02" drill="0.6" shape="square"/>
<pad name="25" x="24.13" y="0" drill="0.6"/>
<pad name="26" x="24.13" y="2.54" drill="0.6"/>
<pad name="27" x="24.13" y="5.08" drill="0.6"/>
<pad name="28" x="24.13" y="7.62" drill="0.6"/>
<pad name="29" x="24.13" y="10.16" drill="0.6"/>
<pad name="30" x="24.13" y="12.7" drill="0.6"/>
<pad name="31" x="24.13" y="15.24" drill="0.6"/>
<pad name="32" x="24.13" y="17.78" drill="0.6"/>
<pad name="33" x="24.13" y="20.32" drill="0.6"/>
<pad name="34" x="24.13" y="22.86" drill="0.6"/>
<pad name="35" x="24.13" y="25.4" drill="0.6"/>
<pad name="36" x="24.13" y="27.94" drill="0.6"/>
<pad name="37" x="24.13" y="30.48" drill="0.6"/>
<pad name="38" x="24.13" y="33.02" drill="0.6"/>
<text x="7.62" y="27.94" size="1.27" layer="27">&gt;Value</text>
<text x="7.62" y="25.4" size="1.27" layer="25">&gt;Name</text>
<pad name="14" x="0" y="0" drill="0.6"/>
<pad name="15" x="0" y="-2.54" drill="0.6"/>
<pad name="16" x="0" y="-5.08" drill="0.6"/>
<pad name="17" x="0" y="-7.62" drill="0.6"/>
<pad name="18" x="0" y="-10.16" drill="0.6"/>
<pad name="19" x="0" y="-12.7" drill="0.6"/>
<pad name="24" x="24.13" y="-2.54" drill="0.6"/>
<pad name="23" x="24.13" y="-5.08" drill="0.6"/>
<pad name="22" x="24.13" y="-7.62" drill="0.6"/>
<pad name="21" x="24.13" y="-10.16" drill="0.6"/>
<pad name="20" x="24.13" y="-12.7" drill="0.6"/>
<wire x1="-2.54" y1="-15.24" x2="7.62" y2="-15.24" width="0.127" layer="21"/>
<wire x1="7.62" y1="-15.24" x2="16.51" y2="-15.24" width="0.127" layer="21"/>
<wire x1="16.51" y1="-15.24" x2="26.67" y2="-15.24" width="0.127" layer="21"/>
<wire x1="26.67" y1="-15.24" x2="26.67" y2="35.56" width="0.127" layer="21"/>
<wire x1="26.67" y1="35.56" x2="-2.54" y2="35.56" width="0.127" layer="21"/>
<wire x1="-2.54" y1="35.56" x2="-2.54" y2="-15.24" width="0.127" layer="21"/>
<wire x1="7.62" y1="-15.24" x2="7.62" y2="-11.43" width="0.127" layer="21"/>
<wire x1="7.62" y1="-11.43" x2="16.51" y2="-11.43" width="0.127" layer="21"/>
<wire x1="16.51" y1="-11.43" x2="16.51" y2="-15.24" width="0.127" layer="21"/>
</package>
</packages>
<symbols>
<symbol name="ESP32_NODEMCU">
<pin name="3.3V" x="-17.78" y="25.4" length="middle" direction="pwr"/>
<pin name="RESET" x="-17.78" y="22.86" length="middle" direction="in"/>
<pin name="GPIO36" x="-17.78" y="20.32" length="middle" direction="in"/>
<pin name="GPIO39" x="-17.78" y="17.78" length="middle" direction="in"/>
<pin name="GPIO34" x="-17.78" y="15.24" length="middle" direction="in"/>
<pin name="GPIO35" x="-17.78" y="12.7" length="middle" direction="in"/>
<pin name="GPIO32" x="-17.78" y="10.16" length="middle"/>
<pin name="GPIO33" x="-17.78" y="7.62" length="middle"/>
<pin name="GPIO25" x="-17.78" y="5.08" length="middle"/>
<pin name="GPIO26" x="-17.78" y="2.54" length="middle"/>
<pin name="GPIO27" x="-17.78" y="0" length="middle"/>
<pin name="GPIO14" x="-17.78" y="-2.54" length="middle"/>
<pin name="GPI012" x="-17.78" y="-5.08" length="middle"/>
<pin name="GND1" x="-17.78" y="-7.62" length="middle" direction="pwr"/>
<pin name="GPIO13" x="-17.78" y="-10.16" length="middle"/>
<pin name="INTERNAL1" x="-17.78" y="-12.7" length="middle" direction="nc"/>
<pin name="INTERNAL2" x="-17.78" y="-15.24" length="middle" direction="nc"/>
<pin name="INTERNAL3" x="-17.78" y="-17.78" length="middle" direction="nc"/>
<pin name="5V" x="-17.78" y="-20.32" length="middle" direction="pwr"/>
<pin name="INTERNAL4" x="25.4" y="-20.32" length="middle" direction="nc" rot="R180"/>
<pin name="INTERNAL5" x="25.4" y="-17.78" length="middle" direction="nc" rot="R180"/>
<pin name="INTERNAL6" x="25.4" y="-15.24" length="middle" direction="nc" rot="R180"/>
<pin name="GPIO15" x="25.4" y="-12.7" length="middle" rot="R180"/>
<pin name="GPIO2" x="25.4" y="-10.16" length="middle" rot="R180"/>
<pin name="GPIO0" x="25.4" y="-7.62" length="middle" direction="out" rot="R180"/>
<pin name="GPIO4" x="25.4" y="-5.08" length="middle" rot="R180"/>
<pin name="GPIO16" x="25.4" y="-2.54" length="middle" rot="R180"/>
<pin name="GPIO17" x="25.4" y="0" length="middle" rot="R180"/>
<pin name="GPIO5" x="25.4" y="2.54" length="middle" rot="R180"/>
<pin name="GPIO18" x="25.4" y="5.08" length="middle" rot="R180"/>
<pin name="GPIO19" x="25.4" y="7.62" length="middle" rot="R180"/>
<pin name="GND2" x="25.4" y="10.16" length="middle" direction="pwr" rot="R180"/>
<pin name="GPIO21" x="25.4" y="12.7" length="middle" rot="R180"/>
<pin name="INTERNAL7" x="25.4" y="15.24" length="middle" direction="nc" rot="R180"/>
<pin name="INTERNAL8" x="25.4" y="17.78" length="middle" direction="nc" rot="R180"/>
<pin name="GPIO22" x="25.4" y="20.32" length="middle" rot="R180"/>
<pin name="GPIO23" x="25.4" y="22.86" length="middle" rot="R180"/>
<pin name="GND3" x="25.4" y="25.4" length="middle" direction="pwr" rot="R180"/>
<wire x1="-12.7" y1="27.94" x2="-12.7" y2="-25.4" width="0.254" layer="94"/>
<wire x1="-12.7" y1="-25.4" x2="-2.54" y2="-25.4" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-25.4" x2="7.62" y2="-25.4" width="0.254" layer="94"/>
<wire x1="7.62" y1="-25.4" x2="20.32" y2="-25.4" width="0.254" layer="94"/>
<wire x1="20.32" y1="-25.4" x2="20.32" y2="27.94" width="0.254" layer="94"/>
<wire x1="20.32" y1="27.94" x2="-12.7" y2="27.94" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-25.4" x2="-2.54" y2="-20.32" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-20.32" x2="7.62" y2="-20.32" width="0.254" layer="94"/>
<wire x1="7.62" y1="-20.32" x2="7.62" y2="-25.4" width="0.254" layer="94"/>
<text x="-10.16" y="30.48" size="1.778" layer="96">&gt;VALUE</text>
<text x="-10.16" y="33.02" size="1.778" layer="95">&gt;NAME</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="ESP32-NODEMCU" prefix="ESP" uservalue="yes">
<gates>
<gate name="G$1" symbol="ESP32_NODEMCU" x="-2.54" y="0"/>
</gates>
<devices>
<device name="" package="ESP32_NODEMCU">
<connects>
<connect gate="G$1" pin="3.3V" pad="1"/>
<connect gate="G$1" pin="5V" pad="19"/>
<connect gate="G$1" pin="GND1" pad="14"/>
<connect gate="G$1" pin="GND2" pad="32"/>
<connect gate="G$1" pin="GND3" pad="38"/>
<connect gate="G$1" pin="GPI012" pad="13"/>
<connect gate="G$1" pin="GPIO0" pad="25"/>
<connect gate="G$1" pin="GPIO13" pad="15"/>
<connect gate="G$1" pin="GPIO14" pad="12"/>
<connect gate="G$1" pin="GPIO15" pad="23"/>
<connect gate="G$1" pin="GPIO16" pad="27"/>
<connect gate="G$1" pin="GPIO17" pad="28"/>
<connect gate="G$1" pin="GPIO18" pad="30"/>
<connect gate="G$1" pin="GPIO19" pad="31"/>
<connect gate="G$1" pin="GPIO2" pad="24"/>
<connect gate="G$1" pin="GPIO21" pad="33"/>
<connect gate="G$1" pin="GPIO22" pad="36"/>
<connect gate="G$1" pin="GPIO23" pad="37"/>
<connect gate="G$1" pin="GPIO25" pad="9"/>
<connect gate="G$1" pin="GPIO26" pad="10"/>
<connect gate="G$1" pin="GPIO27" pad="11"/>
<connect gate="G$1" pin="GPIO32" pad="7"/>
<connect gate="G$1" pin="GPIO33" pad="8"/>
<connect gate="G$1" pin="GPIO34" pad="5"/>
<connect gate="G$1" pin="GPIO35" pad="6"/>
<connect gate="G$1" pin="GPIO36" pad="3"/>
<connect gate="G$1" pin="GPIO39" pad="4"/>
<connect gate="G$1" pin="GPIO4" pad="26"/>
<connect gate="G$1" pin="GPIO5" pad="29"/>
<connect gate="G$1" pin="INTERNAL1" pad="16"/>
<connect gate="G$1" pin="INTERNAL2" pad="17"/>
<connect gate="G$1" pin="INTERNAL3" pad="18"/>
<connect gate="G$1" pin="INTERNAL4" pad="20"/>
<connect gate="G$1" pin="INTERNAL5" pad="21"/>
<connect gate="G$1" pin="INTERNAL6" pad="22"/>
<connect gate="G$1" pin="INTERNAL7" pad="34"/>
<connect gate="G$1" pin="INTERNAL8" pad="35"/>
<connect gate="G$1" pin="RESET" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="v-reg" urn="urn:adsk.eagle:library:409">
<description>&lt;b&gt;Voltage Regulators&lt;/b&gt;&lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="78XXS" urn="urn:adsk.eagle:footprint:30278/1" library_version="5">
<description>&lt;b&gt;VOLTAGE REGULATOR&lt;/b&gt;</description>
<wire x1="4.826" y1="-4.318" x2="5.08" y2="-4.064" width="0.1524" layer="21"/>
<wire x1="4.826" y1="-4.318" x2="-4.826" y2="-4.318" width="0.1524" layer="21"/>
<wire x1="-5.08" y1="-4.064" x2="-4.826" y2="-4.318" width="0.1524" layer="21"/>
<wire x1="5.08" y1="-1.143" x2="5.08" y2="-4.064" width="0.1524" layer="21"/>
<wire x1="-5.08" y1="-4.064" x2="-5.08" y2="-1.143" width="0.1524" layer="21"/>
<circle x="-4.6228" y="-3.7084" radius="0.254" width="0" layer="21"/>
<pad name="IN" x="-2.54" y="-2.54" drill="1.016" shape="long" rot="R90"/>
<pad name="GND" x="0" y="-2.54" drill="1.016" shape="long" rot="R90"/>
<pad name="OUT" x="2.54" y="-2.54" drill="1.016" shape="long" rot="R90"/>
<text x="-5.08" y="-6.0452" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="-5.08" y="-7.62" size="1.27" layer="27" ratio="10">&gt;VALUE</text>
<text x="-0.635" y="-3.81" size="1.27" layer="51" ratio="10">-</text>
<text x="-3.175" y="-3.81" size="1.27" layer="51" ratio="10">I</text>
<text x="1.905" y="-3.81" size="1.27" layer="51" ratio="10">O</text>
<rectangle x1="-5.334" y1="-1.27" x2="-3.429" y2="0" layer="21"/>
<rectangle x1="-3.429" y1="-0.762" x2="-1.651" y2="0" layer="21"/>
<rectangle x1="-1.651" y1="-1.27" x2="-0.889" y2="0" layer="21"/>
<rectangle x1="-0.889" y1="-0.762" x2="0.889" y2="0" layer="21"/>
<rectangle x1="0.889" y1="-1.27" x2="1.651" y2="0" layer="21"/>
<rectangle x1="1.651" y1="-0.762" x2="3.429" y2="0" layer="21"/>
<rectangle x1="3.429" y1="-1.27" x2="5.334" y2="0" layer="21"/>
<rectangle x1="-3.429" y1="-1.27" x2="-1.651" y2="-0.762" layer="51"/>
<rectangle x1="-0.889" y1="-1.27" x2="0.889" y2="-0.762" layer="51"/>
<rectangle x1="1.651" y1="-1.27" x2="3.429" y2="-0.762" layer="51"/>
</package>
</packages>
<packages3d>
<package3d name="78XXS" urn="urn:adsk.eagle:package:30342/1" type="box" library_version="5">
<description>VOLTAGE REGULATOR</description>
<packageinstances>
<packageinstance name="78XXS"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="78XX" urn="urn:adsk.eagle:symbol:30277/1" library_version="5">
<wire x1="-5.08" y1="-5.08" x2="5.08" y2="-5.08" width="0.4064" layer="94"/>
<wire x1="5.08" y1="-5.08" x2="5.08" y2="2.54" width="0.4064" layer="94"/>
<wire x1="5.08" y1="2.54" x2="-5.08" y2="2.54" width="0.4064" layer="94"/>
<wire x1="-5.08" y1="2.54" x2="-5.08" y2="-5.08" width="0.4064" layer="94"/>
<text x="2.54" y="-7.62" size="1.778" layer="95">&gt;NAME</text>
<text x="2.54" y="-10.16" size="1.778" layer="96">&gt;VALUE</text>
<text x="-2.032" y="-4.318" size="1.524" layer="95">GND</text>
<text x="-4.445" y="-0.635" size="1.524" layer="95">IN</text>
<text x="0.635" y="-0.635" size="1.524" layer="95">OUT</text>
<pin name="IN" x="-7.62" y="0" visible="off" length="short" direction="in"/>
<pin name="GND" x="0" y="-7.62" visible="off" length="short" direction="in" rot="R90"/>
<pin name="OUT" x="7.62" y="0" visible="off" length="short" direction="pas" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="78XXS" urn="urn:adsk.eagle:component:30381/2" prefix="IC" uservalue="yes" library_version="5">
<description>&lt;b&gt;VOLTAGE REGULATOR&lt;/b&gt;</description>
<gates>
<gate name="1" symbol="78XX" x="0" y="0"/>
</gates>
<devices>
<device name="" package="78XXS">
<connects>
<connect gate="1" pin="GND" pad="GND"/>
<connect gate="1" pin="IN" pad="IN"/>
<connect gate="1" pin="OUT" pad="OUT"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:30342/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="POPULARITY" value="18" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="_eigene">
<packages>
<package name="DIL08">
<description>&lt;b&gt;Dual In Line Package&lt;/b&gt;</description>
<wire x1="5.08" y1="2.921" x2="-5.08" y2="2.921" width="0.1524" layer="21"/>
<wire x1="-5.08" y1="-2.921" x2="5.08" y2="-2.921" width="0.1524" layer="21"/>
<wire x1="5.08" y1="2.921" x2="5.08" y2="-2.921" width="0.1524" layer="21"/>
<wire x1="-5.08" y1="2.921" x2="-5.08" y2="1.016" width="0.1524" layer="21"/>
<wire x1="-5.08" y1="-2.921" x2="-5.08" y2="-1.016" width="0.1524" layer="21"/>
<wire x1="-5.08" y1="1.016" x2="-5.08" y2="-1.016" width="0.1524" layer="21" curve="-180"/>
<pad name="1" x="-3.81" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="2" x="-1.27" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="7" x="-1.27" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="8" x="-3.81" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="3" x="1.27" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="4" x="3.81" y="-3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="6" x="1.27" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<pad name="5" x="3.81" y="3.81" drill="0.8128" shape="long" rot="R90"/>
<text x="-5.334" y="-2.921" size="1.27" layer="25" ratio="10" rot="R90">&gt;NAME</text>
<text x="-3.556" y="-0.635" size="1.27" layer="27" ratio="10">&gt;VALUE</text>
</package>
</packages>
<symbols>
<symbol name="MCP2562">
<pin name="TXD" x="-15.24" y="20.32" length="middle"/>
<pin name="VSS" x="-15.24" y="17.78" length="middle"/>
<pin name="VDD" x="-15.24" y="15.24" length="middle"/>
<pin name="RXD" x="-15.24" y="12.7" length="middle"/>
<pin name="VIO" x="12.7" y="12.7" length="middle" rot="R180"/>
<pin name="CANL" x="12.7" y="15.24" length="middle" rot="R180"/>
<pin name="CANH" x="12.7" y="17.78" length="middle" rot="R180"/>
<pin name="STBY" x="12.7" y="20.32" length="middle" rot="R180"/>
<wire x1="-10.16" y1="22.86" x2="-10.16" y2="10.16" width="0.254" layer="94"/>
<wire x1="-10.16" y1="10.16" x2="7.62" y2="10.16" width="0.254" layer="94"/>
<wire x1="7.62" y1="10.16" x2="7.62" y2="22.86" width="0.254" layer="94"/>
<wire x1="7.62" y1="22.86" x2="0" y2="22.86" width="0.254" layer="94"/>
<wire x1="0" y1="22.86" x2="-2.54" y2="22.86" width="0.254" layer="94"/>
<wire x1="-2.54" y1="22.86" x2="-10.16" y2="22.86" width="0.254" layer="94"/>
<text x="-10.16" y="27.94" size="1.778" layer="95">&gt;Name</text>
<text x="-10.16" y="25.4" size="1.778" layer="96">&gt;VALUE</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="MCP2562">
<gates>
<gate name="G$1" symbol="MCP2562" x="10.16" y="-10.16"/>
</gates>
<devices>
<device name="" package="DIL08">
<connects>
<connect gate="G$1" pin="CANH" pad="7"/>
<connect gate="G$1" pin="CANL" pad="6"/>
<connect gate="G$1" pin="RXD" pad="4"/>
<connect gate="G$1" pin="STBY" pad="8"/>
<connect gate="G$1" pin="TXD" pad="1"/>
<connect gate="G$1" pin="VDD" pad="3"/>
<connect gate="G$1" pin="VIO" pad="5"/>
<connect gate="G$1" pin="VSS" pad="2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="frames" urn="urn:adsk.eagle:library:229">
<description>&lt;b&gt;Frames for Sheet and Layout&lt;/b&gt;</description>
<packages>
</packages>
<symbols>
<symbol name="DINA4_L" urn="urn:adsk.eagle:symbol:13867/1" library_version="1">
<frame x1="0" y1="0" x2="264.16" y2="180.34" columns="4" rows="4" layer="94" border-left="no" border-top="no" border-right="no" border-bottom="no"/>
</symbol>
<symbol name="DOCFIELD" urn="urn:adsk.eagle:symbol:13864/1" library_version="1">
<wire x1="0" y1="0" x2="71.12" y2="0" width="0.1016" layer="94"/>
<wire x1="101.6" y1="15.24" x2="87.63" y2="15.24" width="0.1016" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="5.08" width="0.1016" layer="94"/>
<wire x1="0" y1="5.08" x2="71.12" y2="5.08" width="0.1016" layer="94"/>
<wire x1="0" y1="5.08" x2="0" y2="15.24" width="0.1016" layer="94"/>
<wire x1="101.6" y1="15.24" x2="101.6" y2="5.08" width="0.1016" layer="94"/>
<wire x1="71.12" y1="5.08" x2="71.12" y2="0" width="0.1016" layer="94"/>
<wire x1="71.12" y1="5.08" x2="87.63" y2="5.08" width="0.1016" layer="94"/>
<wire x1="71.12" y1="0" x2="101.6" y2="0" width="0.1016" layer="94"/>
<wire x1="87.63" y1="15.24" x2="87.63" y2="5.08" width="0.1016" layer="94"/>
<wire x1="87.63" y1="15.24" x2="0" y2="15.24" width="0.1016" layer="94"/>
<wire x1="87.63" y1="5.08" x2="101.6" y2="5.08" width="0.1016" layer="94"/>
<wire x1="101.6" y1="5.08" x2="101.6" y2="0" width="0.1016" layer="94"/>
<wire x1="0" y1="15.24" x2="0" y2="22.86" width="0.1016" layer="94"/>
<wire x1="101.6" y1="35.56" x2="0" y2="35.56" width="0.1016" layer="94"/>
<wire x1="101.6" y1="35.56" x2="101.6" y2="22.86" width="0.1016" layer="94"/>
<wire x1="0" y1="22.86" x2="101.6" y2="22.86" width="0.1016" layer="94"/>
<wire x1="0" y1="22.86" x2="0" y2="35.56" width="0.1016" layer="94"/>
<wire x1="101.6" y1="22.86" x2="101.6" y2="15.24" width="0.1016" layer="94"/>
<text x="1.27" y="1.27" size="2.54" layer="94">Date:</text>
<text x="12.7" y="1.27" size="2.54" layer="94">&gt;LAST_DATE_TIME</text>
<text x="72.39" y="1.27" size="2.54" layer="94">Sheet:</text>
<text x="86.36" y="1.27" size="2.54" layer="94">&gt;SHEET</text>
<text x="88.9" y="11.43" size="2.54" layer="94">REV:</text>
<text x="1.27" y="19.05" size="2.54" layer="94">TITLE:</text>
<text x="1.27" y="11.43" size="2.54" layer="94">Document Number:</text>
<text x="17.78" y="19.05" size="2.54" layer="94">&gt;DRAWING_NAME</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="DINA4_L" urn="urn:adsk.eagle:component:13919/1" prefix="FRAME" uservalue="yes" library_version="1">
<description>&lt;b&gt;FRAME&lt;/b&gt;&lt;p&gt;
DIN A4, landscape with extra doc field</description>
<gates>
<gate name="G$1" symbol="DINA4_L" x="0" y="0"/>
<gate name="G$2" symbol="DOCFIELD" x="162.56" y="0" addlevel="must"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="diode" urn="urn:adsk.eagle:library:210">
<description>&lt;b&gt;Diodes&lt;/b&gt;&lt;p&gt;
Based on the following sources:
&lt;ul&gt;
&lt;li&gt;Motorola : www.onsemi.com
&lt;li&gt;Fairchild : www.fairchildsemi.com
&lt;li&gt;Philips : www.semiconductors.com
&lt;li&gt;Vishay : www.vishay.de
&lt;/ul&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="DO41-10" urn="urn:adsk.eagle:footprint:43094/1" library_version="8">
<description>&lt;B&gt;DIODE&lt;/B&gt;&lt;p&gt;
diameter 2.54 mm, horizontal, grid 10.16 mm</description>
<wire x1="2.032" y1="-1.27" x2="-2.032" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="2.032" y1="-1.27" x2="2.032" y2="1.27" width="0.1524" layer="21"/>
<wire x1="-2.032" y1="1.27" x2="2.032" y2="1.27" width="0.1524" layer="21"/>
<wire x1="-2.032" y1="1.27" x2="-2.032" y2="-1.27" width="0.1524" layer="21"/>
<wire x1="5.08" y1="0" x2="4.064" y2="0" width="0.762" layer="51"/>
<wire x1="-5.08" y1="0" x2="-4.064" y2="0" width="0.762" layer="51"/>
<wire x1="-0.635" y1="0" x2="0" y2="0" width="0.1524" layer="21"/>
<wire x1="1.016" y1="0.635" x2="1.016" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="1.016" y1="-0.635" x2="0" y2="0" width="0.1524" layer="21"/>
<wire x1="0" y1="0" x2="1.524" y2="0" width="0.1524" layer="21"/>
<wire x1="0" y1="0" x2="1.016" y2="0.635" width="0.1524" layer="21"/>
<wire x1="0" y1="0.635" x2="0" y2="0" width="0.1524" layer="21"/>
<wire x1="0" y1="0" x2="0" y2="-0.635" width="0.1524" layer="21"/>
<pad name="A" x="5.08" y="0" drill="1.1176"/>
<pad name="C" x="-5.08" y="0" drill="1.1176"/>
<text x="-2.032" y="1.651" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="-2.032" y="-2.794" size="1.27" layer="27" ratio="10">&gt;VALUE</text>
<rectangle x1="-1.651" y1="-1.27" x2="-1.143" y2="1.27" layer="21"/>
<rectangle x1="2.032" y1="-0.381" x2="3.937" y2="0.381" layer="21"/>
<rectangle x1="-3.937" y1="-0.381" x2="-2.032" y2="0.381" layer="21"/>
</package>
</packages>
<packages3d>
<package3d name="DO41-10" urn="urn:adsk.eagle:package:43336/1" type="box" library_version="8">
<description>DIODE
diameter 2.54 mm, horizontal, grid 10.16 mm</description>
<packageinstances>
<packageinstance name="DO41-10"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="D" urn="urn:adsk.eagle:symbol:43091/2" library_version="8">
<wire x1="-1.27" y1="-1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="-1.27" y2="1.27" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="1.27" x2="-1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="0" x2="-1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="-1.27" y1="0" x2="-2.54" y2="0" width="0.254" layer="94"/>
<wire x1="2.54" y1="0" x2="1.27" y2="0" width="0.254" layer="94"/>
<text x="2.54" y="0.4826" size="1.778" layer="95">&gt;NAME</text>
<text x="2.54" y="-2.3114" size="1.778" layer="96">&gt;VALUE</text>
<text x="-2.54" y="0" size="0.4064" layer="99" align="center">SpiceOrder 1</text>
<text x="2.54" y="0" size="0.4064" layer="99" align="center">SpiceOrder 2</text>
<pin name="A" x="-2.54" y="0" visible="off" length="point" direction="pas"/>
<pin name="C" x="2.54" y="0" visible="off" length="point" direction="pas" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="1N4004" urn="urn:adsk.eagle:component:43462/3" prefix="D" library_version="8">
<description>&lt;B&gt;DIODE&lt;/B&gt;&lt;p&gt;
general purpose rectifier, 1 A</description>
<gates>
<gate name="1" symbol="D" x="0" y="0"/>
</gates>
<devices>
<device name="" package="DO41-10">
<connects>
<connect gate="1" pin="A" pad="A"/>
<connect gate="1" pin="C" pad="C"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:43336/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="POPULARITY" value="82" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="con-ptr500" urn="urn:adsk.eagle:library:181">
<description>&lt;b&gt;PTR Connectors&lt;/b&gt;&lt;p&gt;
Aug. 2004 / PTR Meßtechnik:&lt;br&gt;
Die Bezeichnung der Serie AK505 wurde geändert.&lt;br&gt;
Es handelt sich hierbei um AK500 in horizontaler Ausführung.&lt;p&gt;
&lt;TABLE BORDER=0 CELLSPACING=1 CELLPADDING=2&gt;
  &lt;TR&gt;
    &lt;TD ALIGN=LEFT&gt;
      &lt;FONT SIZE=4 FACE=ARIAL&gt;&lt;B&gt;Alte Bezeichnung&lt;/B&gt;&lt;/FONT&gt;
    &lt;/TD&gt;
    &lt;TD ALIGN=LEFT&gt;
      &lt;FONT SIZE=4 FACE=ARIAL&gt;&lt;B&gt;Neue Bezeichnung&lt;/B&gt;&lt;/FONT&gt;
    &lt;/TD&gt;
  &lt;/TR&gt;
  &lt;TR&gt;
    &lt;TD ALIGN=LEFT&gt;
      &lt;B&gt;
      &lt;FONT SIZE=3 FACE=ARIAL color="#FF0000"&gt;AK505/2,grau&lt;/FONT&gt;
      &lt;/B&gt;
    &lt;/TD&gt;
    &lt;TD ALIGN=LEFT&gt;
      &lt;B&gt;
      &lt;FONT SIZE=3 FACE=ARIAL color="#0000FF"&gt;AK500/2-5.0-H-GRAU&lt;/FONT&gt;
      &lt;/B&gt;
    &lt;/TD&gt;
  &lt;/TR&gt;
  &lt;TR&gt;
    &lt;TD ALIGN=LEFT&gt;
      &lt;B&gt;
      &lt;FONT SIZE=3 FACE=ARIAL color="#FF0000"&gt;AK505/2DS,grau&lt;/FONT&gt;
      &lt;/B&gt;
    &lt;/TD&gt;
    &lt;TD ALIGN=LEFT&gt;
      &lt;B&gt;
      &lt;FONT SIZE=3 FACE=ARIAL color="#0000FF"&gt;AK500/2DS-5.0-H-GRAU&lt;/FONT&gt;
      &lt;/B&gt;
    &lt;/TD&gt;
  &lt;/TR&gt;
  &lt;TR&gt;
    &lt;TD ALIGN=LEFT&gt;
      &lt;B&gt;
      &lt;FONT SIZE=3 FACE=ARIAL color="#FF0000"&gt;AKZ505/2,grau&lt;/FONT&gt;
      &lt;/B&gt;
    &lt;/TD&gt;
    &lt;TD ALIGN=LEFT&gt;
      &lt;B&gt;
      &lt;FONT SIZE=3 FACE=ARIAL color="#0000FF"&gt;AKZ500/2-5.08-H-GRAU&lt;/FONT&gt;
      &lt;/B&gt;
    &lt;/TD&gt;
  &lt;/TABLE&gt;

&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="AK500/5" urn="urn:adsk.eagle:footprint:9857/1" library_version="3">
<description>&lt;b&gt;CONNECTOR&lt;/b&gt;</description>
<wire x1="-12.5222" y1="-3.556" x2="-12.5222" y2="-2.159" width="0.1524" layer="21"/>
<wire x1="-12.5222" y1="-3.556" x2="-2.4892" y2="-3.556" width="0.1524" layer="21"/>
<wire x1="-12.5222" y1="3.048" x2="-13.0302" y2="3.175" width="0.1524" layer="21"/>
<wire x1="-12.5222" y1="3.048" x2="-12.5222" y2="3.937" width="0.1524" layer="21"/>
<wire x1="-13.0302" y1="3.175" x2="-13.0302" y2="2.032" width="0.1524" layer="21"/>
<wire x1="-13.0302" y1="2.032" x2="-12.5222" y2="2.159" width="0.1524" layer="21"/>
<wire x1="-12.5222" y1="2.159" x2="-12.5222" y2="3.048" width="0.1524" layer="21"/>
<wire x1="-12.5222" y1="2.159" x2="-2.4892" y2="2.159" width="0.1524" layer="21"/>
<wire x1="-12.5222" y1="-2.159" x2="-2.4892" y2="-2.159" width="0.1524" layer="21"/>
<wire x1="-12.5222" y1="-2.159" x2="-12.5222" y2="2.159" width="0.1524" layer="21"/>
<wire x1="-6.0198" y1="-1.397" x2="-3.6068" y2="1.016" width="0.1524" layer="51"/>
<wire x1="-6.4008" y1="-1.016" x2="-3.9878" y2="1.397" width="0.1524" layer="51"/>
<wire x1="-11.43" y1="-1.016" x2="-9.017" y2="1.397" width="0.1524" layer="51"/>
<wire x1="-11.049" y1="-1.397" x2="-8.636" y2="1.016" width="0.1524" layer="51"/>
<wire x1="-6.0198" y1="-1.016" x2="-3.9878" y2="1.016" width="0.6096" layer="51"/>
<wire x1="-11.049" y1="-1.016" x2="-9.017" y2="1.016" width="0.6096" layer="51"/>
<wire x1="-6.4008" y1="-1.016" x2="-6.0198" y2="-1.397" width="0.1524" layer="51"/>
<wire x1="-3.9878" y1="1.397" x2="-3.6068" y2="1.016" width="0.1524" layer="51"/>
<wire x1="-11.43" y1="-1.016" x2="-11.049" y2="-1.397" width="0.1524" layer="51"/>
<wire x1="-9.017" y1="1.397" x2="-8.636" y2="1.016" width="0.1524" layer="51"/>
<wire x1="-2.4892" y1="-3.556" x2="-2.4892" y2="-2.159" width="0.1524" layer="21"/>
<wire x1="12.5222" y1="3.937" x2="-2.4892" y2="3.937" width="0.1524" layer="21"/>
<wire x1="-2.4892" y1="2.159" x2="-2.4892" y2="3.937" width="0.1524" layer="21"/>
<wire x1="12.5222" y1="3.937" x2="12.5222" y2="2.159" width="0.1524" layer="21"/>
<wire x1="12.5222" y1="2.159" x2="12.5222" y2="-2.159" width="0.1524" layer="21"/>
<wire x1="-2.4892" y1="-2.159" x2="12.5222" y2="-2.159" width="0.1524" layer="21"/>
<wire x1="-2.4892" y1="-2.159" x2="-2.4892" y2="2.159" width="0.1524" layer="21"/>
<wire x1="12.5222" y1="-2.159" x2="12.5222" y2="-3.556" width="0.1524" layer="21"/>
<wire x1="3.9878" y1="-1.397" x2="6.4008" y2="1.016" width="0.1524" layer="51"/>
<wire x1="3.6068" y1="-1.016" x2="6.0198" y2="1.397" width="0.1524" layer="51"/>
<wire x1="-1.397" y1="-1.016" x2="1.016" y2="1.397" width="0.1524" layer="51"/>
<wire x1="-1.016" y1="-1.397" x2="1.397" y2="1.016" width="0.1524" layer="51"/>
<wire x1="3.9878" y1="-1.016" x2="6.0198" y2="1.016" width="0.6096" layer="51"/>
<wire x1="-1.016" y1="-1.016" x2="1.016" y2="1.016" width="0.6096" layer="51"/>
<wire x1="3.6068" y1="-1.016" x2="3.9878" y2="-1.397" width="0.1524" layer="51"/>
<wire x1="6.0198" y1="1.397" x2="6.4008" y2="1.016" width="0.1524" layer="51"/>
<wire x1="-1.397" y1="-1.016" x2="-1.016" y2="-1.397" width="0.1524" layer="51"/>
<wire x1="1.016" y1="1.397" x2="1.397" y2="1.016" width="0.1524" layer="51"/>
<wire x1="8.9916" y1="-1.016" x2="11.0236" y2="1.016" width="0.6096" layer="51"/>
<wire x1="8.9916" y1="-1.397" x2="11.4046" y2="1.016" width="0.1524" layer="51"/>
<wire x1="8.6106" y1="-1.016" x2="11.0236" y2="1.397" width="0.1524" layer="51"/>
<wire x1="8.6106" y1="-1.016" x2="8.9916" y2="-1.397" width="0.1524" layer="51"/>
<wire x1="11.0236" y1="1.397" x2="11.4046" y2="1.016" width="0.1524" layer="51"/>
<wire x1="-2.4892" y1="-3.556" x2="12.5222" y2="-3.556" width="0.1524" layer="21"/>
<wire x1="-2.4892" y1="2.159" x2="12.5222" y2="2.159" width="0.1524" layer="21"/>
<wire x1="-2.4892" y1="3.937" x2="-12.5222" y2="3.937" width="0.1524" layer="21"/>
<circle x="-5.0038" y="3.048" radius="0.508" width="0.1524" layer="21"/>
<circle x="-10.033" y="3.048" radius="0.508" width="0.1524" layer="21"/>
<circle x="-5.0038" y="0" radius="1.778" width="0.1524" layer="51"/>
<circle x="-10.033" y="0" radius="1.778" width="0.1524" layer="51"/>
<circle x="5.0038" y="3.048" radius="0.508" width="0.1524" layer="21"/>
<circle x="0" y="3.048" radius="0.508" width="0.1524" layer="21"/>
<circle x="5.0038" y="0" radius="1.778" width="0.1524" layer="51"/>
<circle x="0" y="0" radius="1.778" width="0.1524" layer="51"/>
<circle x="10.0076" y="0" radius="1.778" width="0.1524" layer="51"/>
<circle x="10.0076" y="3.048" radius="0.508" width="0.1524" layer="21"/>
<pad name="1" x="-10.033" y="0" drill="1.3208" shape="long" rot="R90"/>
<pad name="2" x="-5.0038" y="0" drill="1.3208" shape="long" rot="R90"/>
<pad name="3" x="0" y="0" drill="1.3208" shape="long" rot="R90"/>
<pad name="4" x="5.0038" y="0" drill="1.3208" shape="long" rot="R90"/>
<pad name="5" x="10.0076" y="0" drill="1.3208" shape="long" rot="R90"/>
<text x="-12.5984" y="4.445" size="1.778" layer="25" ratio="10">&gt;NAME</text>
<text x="-12.5984" y="-5.715" size="1.778" layer="27" ratio="10">&gt;VALUE</text>
<text x="-11.8872" y="2.54" size="0.9906" layer="21" ratio="12">1</text>
<text x="-7.0612" y="2.54" size="0.9906" layer="21" ratio="12">2</text>
<text x="-1.7272" y="2.54" size="0.9906" layer="21" ratio="12">3</text>
<text x="3.0988" y="2.54" size="0.9906" layer="21" ratio="12">4</text>
<text x="8.1788" y="2.54" size="0.9906" layer="21" ratio="12">5</text>
<rectangle x1="-7.8994" y1="-1.905" x2="-7.1374" y2="1.905" layer="21"/>
<rectangle x1="2.1082" y1="-1.905" x2="2.8702" y2="1.905" layer="21"/>
<rectangle x1="7.1374" y1="-1.905" x2="7.8994" y2="1.905" layer="21"/>
</package>
</packages>
<packages3d>
<package3d name="AK500/5" urn="urn:adsk.eagle:package:9890/1" type="box" library_version="3">
<description>CONNECTOR</description>
<packageinstances>
<packageinstance name="AK500/5"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="KL" urn="urn:adsk.eagle:symbol:9788/2" library_version="3">
<circle x="1.27" y="0" radius="1.27" width="0.254" layer="94"/>
<text x="-1.27" y="0.889" size="1.778" layer="95" rot="R180">&gt;NAME</text>
<pin name="KL" x="5.08" y="0" visible="off" length="short" direction="pas" rot="R180"/>
</symbol>
<symbol name="KLV" urn="urn:adsk.eagle:symbol:9842/1" library_version="3">
<circle x="1.27" y="0" radius="1.27" width="0.254" layer="94"/>
<text x="-1.27" y="0.889" size="1.778" layer="95" rot="R180">&gt;NAME</text>
<text x="-3.81" y="-3.683" size="1.778" layer="96">&gt;VALUE</text>
<pin name="KL" x="5.08" y="0" visible="off" length="short" direction="pas" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="AK500/5" urn="urn:adsk.eagle:component:9921/3" prefix="X" uservalue="yes" library_version="3">
<description>&lt;b&gt;CONNECTOR&lt;/b&gt;</description>
<gates>
<gate name="-1" symbol="KL" x="0" y="12.7" addlevel="always"/>
<gate name="-2" symbol="KL" x="0" y="7.62" addlevel="always"/>
<gate name="-3" symbol="KL" x="0" y="2.54" addlevel="always"/>
<gate name="-4" symbol="KL" x="0" y="-2.54" addlevel="always"/>
<gate name="-5" symbol="KLV" x="0" y="-7.62" addlevel="always"/>
</gates>
<devices>
<device name="" package="AK500/5">
<connects>
<connect gate="-1" pin="KL" pad="1"/>
<connect gate="-2" pin="KL" pad="2"/>
<connect gate="-3" pin="KL" pad="3"/>
<connect gate="-4" pin="KL" pad="4"/>
<connect gate="-5" pin="KL" pad="5"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:9890/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="" constant="no"/>
<attribute name="OC_FARNELL" value="unknown" constant="no"/>
<attribute name="OC_NEWARK" value="unknown" constant="no"/>
<attribute name="POPULARITY" value="1" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="ESP1" library="_esp32" deviceset="ESP32-NODEMCU" device="" value="ESP32 NodeMCU"/>
<part name="IC1" library="v-reg" library_urn="urn:adsk.eagle:library:409" deviceset="78XXS" device="" package3d_urn="urn:adsk.eagle:package:30342/1" value="5V"/>
<part name="U$1" library="_eigene" deviceset="MCP2562" device=""/>
<part name="FRAME1" library="frames" library_urn="urn:adsk.eagle:library:229" deviceset="DINA4_L" device=""/>
<part name="D1" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="1N4004" device="" package3d_urn="urn:adsk.eagle:package:43336/1" value="1N4001"/>
<part name="X1" library="con-ptr500" library_urn="urn:adsk.eagle:library:181" deviceset="AK500/5" device="" package3d_urn="urn:adsk.eagle:package:9890/1"/>
</parts>
<sheets>
<sheet>
<plain>
<text x="2.54" y="45.72" size="1.778" layer="97">CANL = blau
CANH = weiss
</text>
<wire x1="55.88" y1="154.94" x2="55.88" y2="137.16" width="0.1524" layer="94"/>
<wire x1="55.88" y1="137.16" x2="71.12" y2="137.16" width="0.1524" layer="94"/>
<wire x1="71.12" y1="137.16" x2="71.12" y2="154.94" width="0.1524" layer="94"/>
<wire x1="71.12" y1="154.94" x2="55.88" y2="154.94" width="0.1524" layer="94"/>
<text x="55.88" y="134.62" size="1.778" layer="95">VL53L0X</text>
<text x="93.98" y="147.32" size="1.778" layer="97">blue</text>
<text x="93.98" y="144.78" size="1.778" layer="97">white</text>
</plain>
<instances>
<instance part="ESP1" gate="G$1" x="180.34" y="114.3" smashed="yes">
<attribute name="VALUE" x="170.18" y="144.78" size="1.778" layer="96"/>
<attribute name="NAME" x="170.18" y="147.32" size="1.778" layer="95"/>
</instance>
<instance part="IC1" gate="1" x="78.74" y="93.98" smashed="yes">
<attribute name="NAME" x="81.28" y="86.36" size="1.778" layer="95"/>
<attribute name="VALUE" x="81.28" y="83.82" size="1.778" layer="96"/>
</instance>
<instance part="U$1" gate="G$1" x="81.28" y="68.58" smashed="yes" rot="R180">
<attribute name="NAME" x="91.44" y="40.64" size="1.778" layer="95" rot="R180"/>
<attribute name="VALUE" x="91.44" y="43.18" size="1.778" layer="96" rot="R180"/>
</instance>
<instance part="FRAME1" gate="G$1" x="0" y="0" smashed="yes"/>
<instance part="FRAME1" gate="G$2" x="162.56" y="0" smashed="yes">
<attribute name="LAST_DATE_TIME" x="175.26" y="1.27" size="2.54" layer="94"/>
<attribute name="SHEET" x="248.92" y="1.27" size="2.54" layer="94"/>
<attribute name="DRAWING_NAME" x="180.34" y="19.05" size="2.54" layer="94"/>
</instance>
<instance part="D1" gate="1" x="58.42" y="93.98" smashed="yes">
<attribute name="NAME" x="60.96" y="94.4626" size="1.778" layer="95"/>
<attribute name="VALUE" x="60.96" y="91.6686" size="1.778" layer="96"/>
</instance>
<instance part="X1" gate="-1" x="27.94" y="93.98" smashed="yes">
<attribute name="NAME" x="26.67" y="94.869" size="1.778" layer="95" rot="R180"/>
</instance>
<instance part="X1" gate="-2" x="27.94" y="88.9" smashed="yes">
<attribute name="NAME" x="26.67" y="89.789" size="1.778" layer="95" rot="R180"/>
</instance>
<instance part="X1" gate="-4" x="27.94" y="53.34" smashed="yes">
<attribute name="NAME" x="26.67" y="54.229" size="1.778" layer="95" rot="R180"/>
</instance>
<instance part="X1" gate="-5" x="27.94" y="48.26" smashed="yes">
<attribute name="NAME" x="26.67" y="49.149" size="1.778" layer="95" rot="R180"/>
<attribute name="VALUE" x="24.13" y="44.577" size="1.778" layer="96"/>
</instance>
</instances>
<busses>
</busses>
<nets>
<net name="N$1" class="0">
<segment>
<wire x1="223.52" y1="109.22" x2="223.52" y2="55.88" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="RXD"/>
<wire x1="223.52" y1="55.88" x2="96.52" y2="55.88" width="0.1524" layer="91"/>
<pinref part="ESP1" gate="G$1" pin="GPIO4"/>
<wire x1="205.74" y1="109.22" x2="223.52" y2="109.22" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$2" class="0">
<segment>
<pinref part="U$1" gate="G$1" pin="TXD"/>
<wire x1="96.52" y1="48.26" x2="226.06" y2="48.26" width="0.1524" layer="91"/>
<wire x1="226.06" y1="48.26" x2="226.06" y2="116.84" width="0.1524" layer="91"/>
<pinref part="ESP1" gate="G$1" pin="GPIO5"/>
<wire x1="205.74" y1="116.84" x2="226.06" y2="116.84" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$3" class="0">
<segment>
<pinref part="ESP1" gate="G$1" pin="5V"/>
<pinref part="IC1" gate="1" pin="OUT"/>
<wire x1="162.56" y1="93.98" x2="104.14" y2="93.98" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="VDD"/>
<wire x1="104.14" y1="93.98" x2="86.36" y2="93.98" width="0.1524" layer="91"/>
<wire x1="96.52" y1="53.34" x2="104.14" y2="53.34" width="0.1524" layer="91"/>
<wire x1="104.14" y1="53.34" x2="104.14" y2="93.98" width="0.1524" layer="91"/>
<junction x="104.14" y="93.98"/>
</segment>
</net>
<net name="GND" class="0">
<segment>
<label x="76.2" y="142.24" size="1.778" layer="95"/>
<wire x1="71.12" y1="142.24" x2="144.78" y2="142.24" width="0.1524" layer="91"/>
<pinref part="IC1" gate="1" pin="GND"/>
<wire x1="78.74" y1="86.36" x2="78.74" y2="76.2" width="0.1524" layer="91"/>
<wire x1="78.74" y1="76.2" x2="60.96" y2="76.2" width="0.1524" layer="91"/>
<wire x1="78.74" y1="76.2" x2="109.22" y2="76.2" width="0.1524" layer="91"/>
<junction x="78.74" y="76.2"/>
<wire x1="109.22" y1="76.2" x2="144.78" y2="76.2" width="0.1524" layer="91"/>
<wire x1="144.78" y1="76.2" x2="144.78" y2="106.68" width="0.1524" layer="91"/>
<pinref part="ESP1" gate="G$1" pin="GND1"/>
<wire x1="144.78" y1="106.68" x2="162.56" y2="106.68" width="0.1524" layer="91"/>
<pinref part="U$1" gate="G$1" pin="VSS"/>
<wire x1="96.52" y1="50.8" x2="109.22" y2="50.8" width="0.1524" layer="91"/>
<wire x1="109.22" y1="50.8" x2="109.22" y2="76.2" width="0.1524" layer="91"/>
<junction x="109.22" y="76.2"/>
<label x="35.56" y="88.9" size="1.778" layer="95"/>
<pinref part="U$1" gate="G$1" pin="STBY"/>
<wire x1="68.58" y1="48.26" x2="60.96" y2="48.26" width="0.1524" layer="91"/>
<wire x1="60.96" y1="76.2" x2="60.96" y2="48.26" width="0.1524" layer="91"/>
<wire x1="60.96" y1="88.9" x2="60.96" y2="76.2" width="0.1524" layer="91"/>
<junction x="60.96" y="76.2"/>
<wire x1="144.78" y1="142.24" x2="144.78" y2="106.68" width="0.1524" layer="91"/>
<junction x="144.78" y="106.68"/>
<wire x1="33.02" y1="88.9" x2="60.96" y2="88.9" width="0.1524" layer="91"/>
<pinref part="X1" gate="-2" pin="KL"/>
</segment>
</net>
<net name="12V" class="0">
<segment>
<label x="35.56" y="93.98" size="1.778" layer="95"/>
<pinref part="D1" gate="1" pin="A"/>
<wire x1="33.02" y1="93.98" x2="55.88" y2="93.98" width="0.1524" layer="91"/>
<pinref part="X1" gate="-1" pin="KL"/>
</segment>
</net>
<net name="CANL" class="0">
<segment>
<pinref part="U$1" gate="G$1" pin="CANL"/>
<wire x1="68.58" y1="53.34" x2="33.02" y2="53.34" width="0.1524" layer="91"/>
<label x="35.56" y="53.34" size="1.778" layer="95"/>
<pinref part="X1" gate="-4" pin="KL"/>
</segment>
</net>
<net name="CANH" class="0">
<segment>
<pinref part="U$1" gate="G$1" pin="CANH"/>
<wire x1="68.58" y1="50.8" x2="50.8" y2="50.8" width="0.1524" layer="91"/>
<label x="35.56" y="48.26" size="1.778" layer="95"/>
<wire x1="50.8" y1="48.26" x2="50.8" y2="50.8" width="0.1524" layer="91"/>
<wire x1="33.02" y1="48.26" x2="50.8" y2="48.26" width="0.1524" layer="91"/>
<pinref part="X1" gate="-5" pin="KL"/>
</segment>
</net>
<net name="SHUT" class="0">
<segment>
<wire x1="71.12" y1="152.4" x2="86.36" y2="152.4" width="0.1524" layer="91"/>
<label x="76.2" y="152.4" size="1.778" layer="95"/>
</segment>
</net>
<net name="INT" class="0">
<segment>
<wire x1="71.12" y1="149.86" x2="86.36" y2="149.86" width="0.1524" layer="91"/>
<label x="76.2" y="149.86" size="1.778" layer="95"/>
</segment>
</net>
<net name="SCL" class="0">
<segment>
<label x="76.2" y="147.32" size="1.778" layer="95"/>
<wire x1="71.12" y1="147.32" x2="106.68" y2="147.32" width="0.1524" layer="91"/>
<wire x1="106.68" y1="147.32" x2="106.68" y2="175.26" width="0.1524" layer="91"/>
<wire x1="106.68" y1="175.26" x2="220.98" y2="175.26" width="0.1524" layer="91"/>
<wire x1="220.98" y1="175.26" x2="220.98" y2="134.62" width="0.1524" layer="91"/>
<pinref part="ESP1" gate="G$1" pin="GPIO22"/>
<wire x1="205.74" y1="134.62" x2="220.98" y2="134.62" width="0.1524" layer="91"/>
<label x="210.82" y="134.62" size="1.778" layer="95"/>
</segment>
</net>
<net name="SDA" class="0">
<segment>
<label x="76.2" y="144.78" size="1.778" layer="95"/>
<wire x1="71.12" y1="144.78" x2="109.22" y2="144.78" width="0.1524" layer="91"/>
<wire x1="109.22" y1="144.78" x2="109.22" y2="170.18" width="0.1524" layer="91"/>
<wire x1="109.22" y1="170.18" x2="228.6" y2="170.18" width="0.1524" layer="91"/>
<wire x1="228.6" y1="170.18" x2="228.6" y2="127" width="0.1524" layer="91"/>
<pinref part="ESP1" gate="G$1" pin="GPIO21"/>
<wire x1="205.74" y1="127" x2="228.6" y2="127" width="0.1524" layer="91"/>
<label x="210.82" y="127" size="1.778" layer="95"/>
</segment>
</net>
<net name="VIN" class="0">
<segment>
<label x="76.2" y="139.7" size="1.778" layer="95"/>
<pinref part="U$1" gate="G$1" pin="VIO"/>
<wire x1="139.7" y1="66.04" x2="68.58" y2="66.04" width="0.1524" layer="91"/>
<wire x1="68.58" y1="66.04" x2="68.58" y2="55.88" width="0.1524" layer="91"/>
<pinref part="ESP1" gate="G$1" pin="3.3V"/>
<wire x1="139.7" y1="66.04" x2="139.7" y2="139.7" width="0.1524" layer="91"/>
<wire x1="139.7" y1="139.7" x2="162.56" y2="139.7" width="0.1524" layer="91"/>
<wire x1="71.12" y1="139.7" x2="139.7" y2="139.7" width="0.1524" layer="91"/>
<junction x="139.7" y="139.7"/>
</segment>
</net>
<net name="N$4" class="0">
<segment>
<pinref part="D1" gate="1" pin="C"/>
<pinref part="IC1" gate="1" pin="IN"/>
<wire x1="60.96" y1="93.98" x2="71.12" y2="93.98" width="0.1524" layer="91"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
<compatibility>
<note version="6.3" minversion="6.2.2" severity="warning">
Since Version 6.2.2 text objects can contain more than one line,
which will not be processed correctly with this version.
</note>
<note version="8.2" severity="warning">
Since Version 8.2, EAGLE supports online libraries. The ids
of those online libraries will not be understood (or retained)
with this version.
</note>
<note version="8.3" severity="warning">
Since Version 8.3, EAGLE supports URNs for individual library
assets (packages, symbols, and devices). The URNs of those assets
will not be understood (or retained) with this version.
</note>
<note version="8.3" severity="warning">
Since Version 8.3, EAGLE supports the association of 3D packages
with devices in libraries, schematics, and board files. Those 3D
packages will not be understood (or retained) with this version.
</note>
</compatibility>
</eagle>
