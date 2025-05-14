<table class="regdef" id="Reg_clkdividx">
 <tr>
  <th class="regdef" colspan=5>
   <div>pdm2pcm.CLKDIVIDX @ 0x0</div>
   <div><p>Control register</p></div>
   <div>Reset default = 0x0, mask 0xffff</div>
  </th>
 </tr>
<tr><td colspan=5><table class="regpic"><tr><td class="bitnum">31</td><td class="bitnum">30</td><td class="bitnum">29</td><td class="bitnum">28</td><td class="bitnum">27</td><td class="bitnum">26</td><td class="bitnum">25</td><td class="bitnum">24</td><td class="bitnum">23</td><td class="bitnum">22</td><td class="bitnum">21</td><td class="bitnum">20</td><td class="bitnum">19</td><td class="bitnum">18</td><td class="bitnum">17</td><td class="bitnum">16</td></tr><tr><td class="unused" colspan=16>&nbsp;</td>
</tr>
<tr><td class="bitnum">15</td><td class="bitnum">14</td><td class="bitnum">13</td><td class="bitnum">12</td><td class="bitnum">11</td><td class="bitnum">10</td><td class="bitnum">9</td><td class="bitnum">8</td><td class="bitnum">7</td><td class="bitnum">6</td><td class="bitnum">5</td><td class="bitnum">4</td><td class="bitnum">3</td><td class="bitnum">2</td><td class="bitnum">1</td><td class="bitnum">0</td></tr><tr><td class="fname" colspan=16>COUNT</td>
</tr></table></td></tr>
<tr><th width=5%>Bits</th><th width=5%>Type</th><th width=5%>Reset</th><th>Name</th><th>Description</th></tr><tr><td class="regbits">15:0</td><td class="regperm">rw</td><td class="regrv">x</td><td class="regfn">COUNT</td><td class="regde"><p>Index at which clock divide.</p></td></table>
<br>
<table class="regdef" id="Reg_control">
 <tr>
  <th class="regdef" colspan=5>
   <div>pdm2pcm.CONTROL @ 0x4</div>
   <div><p>Control register</p></div>
   <div>Reset default = 0x0, mask 0x3</div>
  </th>
 </tr>
<tr><td colspan=5><table class="regpic"><tr><td class="bitnum">31</td><td class="bitnum">30</td><td class="bitnum">29</td><td class="bitnum">28</td><td class="bitnum">27</td><td class="bitnum">26</td><td class="bitnum">25</td><td class="bitnum">24</td><td class="bitnum">23</td><td class="bitnum">22</td><td class="bitnum">21</td><td class="bitnum">20</td><td class="bitnum">19</td><td class="bitnum">18</td><td class="bitnum">17</td><td class="bitnum">16</td></tr><tr><td class="unused" colspan=16>&nbsp;</td>
</tr>
<tr><td class="bitnum">15</td><td class="bitnum">14</td><td class="bitnum">13</td><td class="bitnum">12</td><td class="bitnum">11</td><td class="bitnum">10</td><td class="bitnum">9</td><td class="bitnum">8</td><td class="bitnum">7</td><td class="bitnum">6</td><td class="bitnum">5</td><td class="bitnum">4</td><td class="bitnum">3</td><td class="bitnum">2</td><td class="bitnum">1</td><td class="bitnum">0</td></tr><tr><td class="unused" colspan=14>&nbsp;</td>
<td class="fname" colspan=1 style="font-size:60.0%">CLEAR</td>
<td class="fname" colspan=1 style="font-size:60.0%">ENABL</td>
</tr></table></td></tr>
<tr><th width=5%>Bits</th><th width=5%>Type</th><th width=5%>Reset</th><th>Name</th><th>Description</th></tr><tr><td class="regbits">0</td><td class="regperm">rw</td><td class="regrv">x</td><td class="regfn">ENABL</td><td class="regde"><p>Starts PDM data processing. The FIFO starts to fill with PCM data.</p></td><tr><td class="regbits">1</td><td class="regperm">rw</td><td class="regrv">x</td><td class="regfn">CLEAR</td><td class="regde"><p>Clears the FIFO buffer.</p></td></table>
<br>
<table class="regdef" id="Reg_status">
 <tr>
  <th class="regdef" colspan=5>
   <div>pdm2pcm.STATUS @ 0x8</div>
   <div><p>Status register</p></div>
   <div>Reset default = 0x0, mask 0x7</div>
  </th>
 </tr>
<tr><td colspan=5><table class="regpic"><tr><td class="bitnum">31</td><td class="bitnum">30</td><td class="bitnum">29</td><td class="bitnum">28</td><td class="bitnum">27</td><td class="bitnum">26</td><td class="bitnum">25</td><td class="bitnum">24</td><td class="bitnum">23</td><td class="bitnum">22</td><td class="bitnum">21</td><td class="bitnum">20</td><td class="bitnum">19</td><td class="bitnum">18</td><td class="bitnum">17</td><td class="bitnum">16</td></tr><tr><td class="unused" colspan=16>&nbsp;</td>
</tr>
<tr><td class="bitnum">15</td><td class="bitnum">14</td><td class="bitnum">13</td><td class="bitnum">12</td><td class="bitnum">11</td><td class="bitnum">10</td><td class="bitnum">9</td><td class="bitnum">8</td><td class="bitnum">7</td><td class="bitnum">6</td><td class="bitnum">5</td><td class="bitnum">4</td><td class="bitnum">3</td><td class="bitnum">2</td><td class="bitnum">1</td><td class="bitnum">0</td></tr><tr><td class="unused" colspan=13>&nbsp;</td>
<td class="fname" colspan=1 style="font-size:60.0%">FULLL</td>
<td class="fname" colspan=1 style="font-size:60.0%">REACH</td>
<td class="fname" colspan=1 style="font-size:60.0%">EMPTY</td>
</tr></table></td></tr>
<tr><th width=5%>Bits</th><th width=5%>Type</th><th width=5%>Reset</th><th>Name</th><th>Description</th></tr><tr><td class="regbits">0</td><td class="regperm">ro</td><td class="regrv">x</td><td class="regfn">EMPTY</td><td class="regde"><p>The FIFO buffer is empty.</p></td><tr><td class="regbits">1</td><td class="regperm">ro</td><td class="regrv">x</td><td class="regfn">REACH</td><td class="regde"><p>The FIFO buffer reached the specified quantity of signal taps.</p></td><tr><td class="regbits">2</td><td class="regperm">ro</td><td class="regrv">x</td><td class="regfn">FULLL</td><td class="regde"><p>The FIFO buffer is full.</p></td></table>
<br>
<table class="regdef" id="Reg_reachcount">
 <tr>
  <th class="regdef" colspan=5>
   <div>pdm2pcm.REACHCOUNT @ 0xc</div>
   <div><p>Number of signal taps stored into the FIFO to assert the FILLD bit in the STATUS register.</p></div>
   <div>Reset default = 0x0, mask 0x3f</div>
  </th>
 </tr>
<tr><td colspan=5><table class="regpic"><tr><td class="bitnum">31</td><td class="bitnum">30</td><td class="bitnum">29</td><td class="bitnum">28</td><td class="bitnum">27</td><td class="bitnum">26</td><td class="bitnum">25</td><td class="bitnum">24</td><td class="bitnum">23</td><td class="bitnum">22</td><td class="bitnum">21</td><td class="bitnum">20</td><td class="bitnum">19</td><td class="bitnum">18</td><td class="bitnum">17</td><td class="bitnum">16</td></tr><tr><td class="unused" colspan=16>&nbsp;</td>
</tr>
<tr><td class="bitnum">15</td><td class="bitnum">14</td><td class="bitnum">13</td><td class="bitnum">12</td><td class="bitnum">11</td><td class="bitnum">10</td><td class="bitnum">9</td><td class="bitnum">8</td><td class="bitnum">7</td><td class="bitnum">6</td><td class="bitnum">5</td><td class="bitnum">4</td><td class="bitnum">3</td><td class="bitnum">2</td><td class="bitnum">1</td><td class="bitnum">0</td></tr><tr><td class="unused" colspan=10>&nbsp;</td>
<td class="fname" colspan=6>COUNT</td>
</tr></table></td></tr>
<tr><th width=5%>Bits</th><th width=5%>Type</th><th width=5%>Reset</th><th>Name</th><th>Description</th></tr><tr><td class="regbits">5:0</td><td class="regperm">rw</td><td class="regrv">x</td><td class="regfn">COUNT</td><td class="regde"><p>Set the signal taps count</p></td></table>
<br>
<table class="regdef" id="Reg_decimcic">
 <tr>
  <th class="regdef" colspan=5>
   <div>pdm2pcm.DECIMCIC @ 0x10</div>
   <div><p>Samples count after which to decimate in the CIC filter.</p></div>
   <div>Reset default = 0x0, mask 0xf</div>
  </th>
 </tr>
<tr><td colspan=5><table class="regpic"><tr><td class="bitnum">31</td><td class="bitnum">30</td><td class="bitnum">29</td><td class="bitnum">28</td><td class="bitnum">27</td><td class="bitnum">26</td><td class="bitnum">25</td><td class="bitnum">24</td><td class="bitnum">23</td><td class="bitnum">22</td><td class="bitnum">21</td><td class="bitnum">20</td><td class="bitnum">19</td><td class="bitnum">18</td><td class="bitnum">17</td><td class="bitnum">16</td></tr><tr><td class="unused" colspan=16>&nbsp;</td>
</tr>
<tr><td class="bitnum">15</td><td class="bitnum">14</td><td class="bitnum">13</td><td class="bitnum">12</td><td class="bitnum">11</td><td class="bitnum">10</td><td class="bitnum">9</td><td class="bitnum">8</td><td class="bitnum">7</td><td class="bitnum">6</td><td class="bitnum">5</td><td class="bitnum">4</td><td class="bitnum">3</td><td class="bitnum">2</td><td class="bitnum">1</td><td class="bitnum">0</td></tr><tr><td class="unused" colspan=12>&nbsp;</td>
<td class="fname" colspan=4>COUNT</td>
</tr></table></td></tr>
<tr><th width=5%>Bits</th><th width=5%>Type</th><th width=5%>Reset</th><th>Name</th><th>Description</th></tr><tr><td class="regbits">3:0</td><td class="regperm">rw</td><td class="regrv">x</td><td class="regfn">COUNT</td><td class="regde"><p>Set the samples count</p></td></table>
<br>
<table class="regdef" id="Reg_rxdata">
  <tr>
    <th class="regdef">
      <div>pdm2pcm.RXDATA @ + 0x14</div>
      <div>1 item ro window</div>
      <div>Byte writes are <i>not</i> supported</div>
    </th>
  </tr>
<tr><td><table class="regpic"><tr><td width="10%"></td><td class="bitnum">31</td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum"></td><td class="bitnum">0</td></tr><tr><td class="regbits">+0x14</td><td class="fname" colspan=32>&nbsp;</td>
</tr><tr><td class="regbits">+0x18</td><td class="fname" colspan=32>&nbsp;</td>
</tr><tr><td>&nbsp;</td><td align=center colspan=32>...</td></tr><tr><td class="regbits">+0x10</td><td class="fname" colspan=32>&nbsp;</td>
</tr><tr><td class="regbits">+0x14</td><td class="fname" colspan=32>&nbsp;</td>
</tr></td></tr></table><tr><td class="regde"><p>PCM Receive data</p></td></tr></table>
<br>
