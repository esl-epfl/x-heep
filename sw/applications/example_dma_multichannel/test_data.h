#ifndef __TEST_DATA_H__
#define __TEST_DATA_H__

#define SIZE_IN_D1 15
#define SIZE_IN_D2 15
#define DMA_DATA_TYPE DMA_DATA_TYPE_WORD

/* 
 * Change the input datatype depending on the DMA_DATA_TYPE.
 * The test data has been generated using byte as datatype, so it's possible to use both uint8_t, uint16_t and uint32_t.
 * NOTE: To fully evaluate the performance gains of the DMA it's recommended to increase the number of memory banks and
 * the size of the data to be copied up ot at least 50x50.
 */
typedef uint32_t dma_input_data_type;

dma_input_data_type test_data[SIZE_IN_D1 * SIZE_IN_D2] = {
    236 ,19 ,35 ,109 ,59 ,58 ,216 ,210 ,72 ,169 ,123 ,74 ,132 ,27 ,208,
    43 ,93 ,216 ,145 ,94 ,235 ,80 ,25 ,146 ,127 ,115 ,9 ,45 ,61 ,60,
    78 ,218 ,28 ,230 ,140 ,236 ,152 ,242 ,98 ,173 ,20 ,206 ,176 ,157 ,219,
    225 ,182 ,118 ,22 ,153 ,196 ,7 ,9 ,87 ,99 ,173 ,11 ,103 ,173 ,49,
    122 ,185 ,128 ,93 ,243 ,29 ,172 ,32 ,207 ,64 ,184 ,40 ,165 ,87 ,157,
    182 ,196 ,160 ,93 ,182 ,91 ,20 ,82 ,156 ,45 ,109 ,132 ,168 ,234 ,85,
    121 ,39 ,188 ,109 ,190 ,127 ,192 ,124 ,149 ,90 ,247 ,68 ,26 ,82 ,57,
    189 ,249 ,91 ,193 ,23 ,210 ,167 ,79 ,175 ,31 ,212 ,131 ,102 ,43 ,4,
    192 ,184 ,234 ,9 ,81 ,4 ,221 ,12 ,181 ,76 ,64 ,131 ,59 ,26 ,83,
    86 ,140 ,176 ,18 ,120 ,115 ,153 ,50 ,41 ,83 ,110 ,157 ,173 ,145 ,81,
    9 ,120 ,250 ,3 ,27 ,119 ,64 ,195 ,48 ,245 ,160 ,78 ,177 ,246 ,59,
    156 ,114 ,97 ,7 ,11 ,224 ,75 ,154 ,223 ,221 ,90 ,77 ,254 ,181 ,198,
    193 ,254 ,161 ,177 ,201 ,3 ,182 ,18 ,36 ,23 ,67 ,46 ,116 ,229 ,166,
    90 ,158 ,248 ,194 ,224 ,218 ,245 ,34 ,60 ,126 ,68 ,248 ,143 ,232 ,226,
    58 ,202 ,172 ,209 ,229 ,89 ,246 ,184 ,215 ,7 ,163 ,204 ,43 ,116 ,33};
;

#endif