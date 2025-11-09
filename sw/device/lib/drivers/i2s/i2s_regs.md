<table class="regdef" id="Reg_control">
 <tr>
  <th class="regdef" colspan=5>
   <div>i2s.CONTROL @ 0x0</div>
   <div><p>control register</p></div>
   <div>Reset default = 0x300, mask 0xfff</div>
  </th>
 </tr>
<tr><td colspan=5><table class="regpic"><tr><td class="bitnum">31</td><td class="bitnum">30</td><td class="bitnum">29</td><td class="bitnum">28</td><td class="bitnum">27</td><td class="bitnum">26</td><td class="bitnum">25</td><td class="bitnum">24</td><td class="bitnum">23</td><td class="bitnum">22</td><td class="bitnum">21</td><td class="bitnum">20</td><td class="bitnum">19</td><td class="bitnum">18</td><td class="bitnum">17</td><td class="bitnum">16</td></tr><tr><td class="unused" colspan=16>&nbsp;</td>
</tr>
<tr><td class="bitnum">15</td><td class="bitnum">14</td><td class="bitnum">13</td><td class="bitnum">12</td><td class="bitnum">11</td><td class="bitnum">10</td><td class="bitnum">9</td><td class="bitnum">8</td><td class="bitnum">7</td><td class="bitnum">6</td><td class="bitnum">5</td><td class="bitnum">4</td><td class="bitnum">3</td><td class="bitnum">2</td><td class="bitnum">1</td><td class="bitnum">0</td></tr><tr><td class="unused" colspan=4>&nbsp;</td>
<td class="fname" colspan=1 style="font-size:17.647058823529413%">RESET_RX_OVERFLOW</td>
<td class="fname" colspan=1 style="font-size:18.75%">RX_START_CHANNEL</td>
<td class="fname" colspan=2 style="font-size:60.0%">DATA_WIDTH</td>
<td class="fname" colspan=1 style="font-size:60.0%">EN_IO</td>
<td class="fname" colspan=1 style="font-size:20.0%">RESET_WATERMARK</td>
<td class="fname" colspan=1 style="font-size:25.0%">EN_WATERMARK</td>
<td class="fname" colspan=1 style="font-size:42.857142857142854%">INTR_EN</td>
<td class="fname" colspan=2>EN_RX</td>
<td class="fname" colspan=1 style="font-size:60.0%">EN_WS</td>
<td class="fname" colspan=1>EN</td>
</tr></table></td></tr>
<tr><th width=5%>Bits</th><th width=5%>Type</th><th width=5%>Reset</th><th>Name</th><th>Description</th></tr><tr><td class="regbits">0</td><td class="regperm">rw</td><td class="regrv">x</td><td class="regfn">EN</td><td class="regde"><p>Enable I2s - CLK Domain</p></td><tr><td class="regbits">1</td><td class="regperm">rw</td><td class="regrv">x</td><td class="regfn">EN_WS</td><td class="regde"><p>Enable word select generation</p></td><tr><td class="regbits">3:2</td><td class="regperm">rw</td><td class="regrv">0x0</td><td class="regfn">EN_RX</td><td class="regde"><p>Enable rx channels</p><table><tr><td>0</td><td>DISABLED</td><td><p>Disable I2s</p></td></tr>
<tr><td>1</td><td>ONLY_LEFT</td><td><p>Enable left channel</p></td></tr>
<tr><td>2</td><td>ONLY_RIGHT</td><td><p>Enable right channel</p></td></tr>
<tr><td>3</td><td>BOTH_CHANNELS</td><td><p>Enable both channels</p></td></tr>
</table></td><tr><td class="regbits">4</td><td class="regperm">rw</td><td class="regrv">x</td><td class="regfn">INTR_EN</td><td class="regde"><p>enable watermark interrupt</p></td><tr><td class="regbits">5</td><td class="regperm">rw</td><td class="regrv">x</td><td class="regfn">EN_WATERMARK</td><td class="regde"><p>en watermark counter</p></td><tr><td class="regbits">6</td><td class="regperm">rw</td><td class="regrv">x</td><td class="regfn">RESET_WATERMARK</td><td class="regde"><p>reset watermark counter</p></td><tr><td class="regbits">7</td><td class="regperm">rw</td><td class="regrv">x</td><td class="regfn">EN_IO</td><td class="regde"><p>connects the peripheral to the IOs</p></td><tr><td class="regbits">9:8</td><td class="regperm">rw</td><td class="regrv">0x3</td><td class="regfn">DATA_WIDTH</td><td class="regde"><p>Bytes per sample</p><table><tr><td>0</td><td>8_BITS</td><td><p>8 bits</p></td></tr>
<tr><td>1</td><td>16_BITS</td><td><p>16 bits</p></td></tr>
<tr><td>2</td><td>24_BITS</td><td><p>24 bits</p></td></tr>
<tr><td>3</td><td>32_BITS</td><td><p>32 bits</p></td></tr>
</table></td><tr><td class="regbits">10</td><td class="regperm">rw</td><td class="regrv">0x0</td><td class="regfn">RX_START_CHANNEL</td><td class="regde"><p>Channel (left/right) of first sample - alternating afterwards.</p><table><tr><td>0</td><td>LEFT_FIRST</td><td><p>Start left channel first (default for WAVE)</p></td></tr>
<tr><td>1</td><td>RIGHT_FIRST</td><td><p>Start right channel first</p></td></tr>
</table></td><tr><td class="regbits">11</td><td class="regperm">rw</td><td class="regrv">x</td><td class="regfn">RESET_RX_OVERFLOW</td><td class="regde"><p>reset rx overflow</p></td></table>
<br>
<table class="regdef" id="Reg_status">
 <tr>
  <th class="regdef" colspan=5>
   <div>i2s.STATUS @ 0x4</div>
   <div><p>Status flags of the I2s peripheral</p></div>
   <div>Reset default = 0x0, mask 0x7</div>
  </th>
 </tr>
<tr><td colspan=5><table class="regpic"><tr><td class="bitnum">31</td><td class="bitnum">30</td><td class="bitnum">29</td><td class="bitnum">28</td><td class="bitnum">27</td><td class="bitnum">26</td><td class="bitnum">25</td><td class="bitnum">24</td><td class="bitnum">23</td><td class="bitnum">22</td><td class="bitnum">21</td><td class="bitnum">20</td><td class="bitnum">19</td><td class="bitnum">18</td><td class="bitnum">17</td><td class="bitnum">16</td></tr><tr><td class="unused" colspan=16>&nbsp;</td>
</tr>
<tr><td class="bitnum">15</td><td class="bitnum">14</td><td class="bitnum">13</td><td class="bitnum">12</td><td class="bitnum">11</td><td class="bitnum">10</td><td class="bitnum">9</td><td class="bitnum">8</td><td class="bitnum">7</td><td class="bitnum">6</td><td class="bitnum">5</td><td class="bitnum">4</td><td class="bitnum">3</td><td class="bitnum">2</td><td class="bitnum">1</td><td class="bitnum">0</td></tr><tr><td class="unused" colspan=13>&nbsp;</td>
<td class="fname" colspan=1 style="font-size:27.272727272727273%">RX_OVERFLOW</td>
<td class="fname" colspan=1 style="font-size:23.076923076923077%">RX_DATA_READY</td>
<td class="fname" colspan=1 style="font-size:42.857142857142854%">RUNNING</td>
</tr></table></td></tr>
<tr><th width=5%>Bits</th><th width=5%>Type</th><th width=5%>Reset</th><th>Name</th><th>Description</th></tr><tr><td class="regbits">0</td><td class="regperm">ro</td><td class="regrv">x</td><td class="regfn">RUNNING</td><td class="regde"><p>1 to indicate that SCK is on</p></td><tr><td class="regbits">1</td><td class="regperm">ro</td><td class="regrv">x</td><td class="regfn">RX_DATA_READY</td><td class="regde"><p>1 to indicate that an RX sample is ready</p></td><tr><td class="regbits">2</td><td class="regperm">ro</td><td class="regrv">x</td><td class="regfn">RX_OVERFLOW</td><td class="regde"><p>1 to indicate that an RX happend - disable rx_channel to clear</p></td></table>
<br>
<table class="regdef" id="Reg_clkdividx">
 <tr>
  <th class="regdef" colspan=5>
   <div>i2s.CLKDIVIDX @ 0x8</div>
   <div><p>Control register</p></div>
   <div>Reset default = 0x4, mask 0xffff</div>
  </th>
 </tr>
<tr><td colspan=5><table class="regpic"><tr><td class="bitnum">31</td><td class="bitnum">30</td><td class="bitnum">29</td><td class="bitnum">28</td><td class="bitnum">27</td><td class="bitnum">26</td><td class="bitnum">25</td><td class="bitnum">24</td><td class="bitnum">23</td><td class="bitnum">22</td><td class="bitnum">21</td><td class="bitnum">20</td><td class="bitnum">19</td><td class="bitnum">18</td><td class="bitnum">17</td><td class="bitnum">16</td></tr><tr><td class="unused" colspan=16>&nbsp;</td>
</tr>
<tr><td class="bitnum">15</td><td class="bitnum">14</td><td class="bitnum">13</td><td class="bitnum">12</td><td class="bitnum">11</td><td class="bitnum">10</td><td class="bitnum">9</td><td class="bitnum">8</td><td class="bitnum">7</td><td class="bitnum">6</td><td class="bitnum">5</td><td class="bitnum">4</td><td class="bitnum">3</td><td class="bitnum">2</td><td class="bitnum">1</td><td class="bitnum">0</td></tr><tr><td class="fname" colspan=16>COUNT</td>
</tr></table></td></tr>
<tr><th width=5%>Bits</th><th width=5%>Type</th><th width=5%>Reset</th><th>Name</th><th>Description</th></tr><tr><td class="regbits">15:0</td><td class="regperm">rw</td><td class="regrv">0x4</td><td class="regfn">COUNT</td><td class="regde"><p>Index at which clock divide.</p></td></table>
<br>
<table class="regdef" id="Reg_rxdata">
 <tr>
  <th class="regdef" colspan=5>
   <div>i2s.RXDATA @ 0xc</div>
   <div><p>I2s Receive data</p></div>
   <div>Reset default = 0x0, mask 0xffffffff</div>
  </th>
 </tr>
<tr><td colspan=5><table class="regpic"><tr><td class="bitnum">31</td><td class="bitnum">30</td><td class="bitnum">29</td><td class="bitnum">28</td><td class="bitnum">27</td><td class="bitnum">26</td><td class="bitnum">25</td><td class="bitnum">24</td><td class="bitnum">23</td><td class="bitnum">22</td><td class="bitnum">21</td><td class="bitnum">20</td><td class="bitnum">19</td><td class="bitnum">18</td><td class="bitnum">17</td><td class="bitnum">16</td></tr><tr><td class="fname" colspan=16>RXDATA...</td>
</tr>
<tr><td class="bitnum">15</td><td class="bitnum">14</td><td class="bitnum">13</td><td class="bitnum">12</td><td class="bitnum">11</td><td class="bitnum">10</td><td class="bitnum">9</td><td class="bitnum">8</td><td class="bitnum">7</td><td class="bitnum">6</td><td class="bitnum">5</td><td class="bitnum">4</td><td class="bitnum">3</td><td class="bitnum">2</td><td class="bitnum">1</td><td class="bitnum">0</td></tr><tr><td class="fname" colspan=16>...RXDATA</td>
</tr></table></td></tr>
<tr><th width=5%>Bits</th><th width=5%>Type</th><th width=5%>Reset</th><th>Name</th><th>Description</th></tr><tr><td class="regbits">31:0</td><td class="regperm">ro</td><td class="regrv">x</td><td class="regfn">RXDATA</td><td class="regde"><p>latest rx data if DATA_READY flag is set</p></td></table>
<br>
<table class="regdef" id="Reg_watermark">
 <tr>
  <th class="regdef" colspan=5>
   <div>i2s.WATERMARK @ 0x10</div>
   <div><p>Watermark to reach for an interrupt</p></div>
   <div>Reset default = 0x0, mask 0xffff</div>
  </th>
 </tr>
<tr><td colspan=5><table class="regpic"><tr><td class="bitnum">31</td><td class="bitnum">30</td><td class="bitnum">29</td><td class="bitnum">28</td><td class="bitnum">27</td><td class="bitnum">26</td><td class="bitnum">25</td><td class="bitnum">24</td><td class="bitnum">23</td><td class="bitnum">22</td><td class="bitnum">21</td><td class="bitnum">20</td><td class="bitnum">19</td><td class="bitnum">18</td><td class="bitnum">17</td><td class="bitnum">16</td></tr><tr><td class="unused" colspan=16>&nbsp;</td>
</tr>
<tr><td class="bitnum">15</td><td class="bitnum">14</td><td class="bitnum">13</td><td class="bitnum">12</td><td class="bitnum">11</td><td class="bitnum">10</td><td class="bitnum">9</td><td class="bitnum">8</td><td class="bitnum">7</td><td class="bitnum">6</td><td class="bitnum">5</td><td class="bitnum">4</td><td class="bitnum">3</td><td class="bitnum">2</td><td class="bitnum">1</td><td class="bitnum">0</td></tr><tr><td class="fname" colspan=16>Watermark</td>
</tr></table></td></tr>
<tr><th width=5%>Bits</th><th width=5%>Type</th><th width=5%>Reset</th><th>Name</th><th>Description</th></tr><tr><td class="regbits">15:0</td><td class="regperm">rw</td><td class="regrv">x</td><td class="regfn">Watermark</td><td class="regde"><p>Count of RX samples written to memory which should trigger an interrupt</p></td></table>
<br>
<table class="regdef" id="Reg_waterlevel">
 <tr>
  <th class="regdef" colspan=5>
   <div>i2s.WATERLEVEL @ 0x14</div>
   <div><p>Watermark counter level</p></div>
   <div>Reset default = 0x0, mask 0xffff</div>
  </th>
 </tr>
<tr><td colspan=5><table class="regpic"><tr><td class="bitnum">31</td><td class="bitnum">30</td><td class="bitnum">29</td><td class="bitnum">28</td><td class="bitnum">27</td><td class="bitnum">26</td><td class="bitnum">25</td><td class="bitnum">24</td><td class="bitnum">23</td><td class="bitnum">22</td><td class="bitnum">21</td><td class="bitnum">20</td><td class="bitnum">19</td><td class="bitnum">18</td><td class="bitnum">17</td><td class="bitnum">16</td></tr><tr><td class="unused" colspan=16>&nbsp;</td>
</tr>
<tr><td class="bitnum">15</td><td class="bitnum">14</td><td class="bitnum">13</td><td class="bitnum">12</td><td class="bitnum">11</td><td class="bitnum">10</td><td class="bitnum">9</td><td class="bitnum">8</td><td class="bitnum">7</td><td class="bitnum">6</td><td class="bitnum">5</td><td class="bitnum">4</td><td class="bitnum">3</td><td class="bitnum">2</td><td class="bitnum">1</td><td class="bitnum">0</td></tr><tr><td class="fname" colspan=16>Waterlevel</td>
</tr></table></td></tr>
<tr><th width=5%>Bits</th><th width=5%>Type</th><th width=5%>Reset</th><th>Name</th><th>Description</th></tr><tr><td class="regbits">15:0</td><td class="regperm">ro</td><td class="regrv">x</td><td class="regfn">Waterlevel</td><td class="regde"><p>Count of RX samples</p></td></table>
<br>
