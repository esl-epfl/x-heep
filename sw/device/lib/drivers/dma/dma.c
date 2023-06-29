/*
                              *******************
******************************* C SOURCE FILE *****************************
**                            *******************                          
**                                                                         
** project  : X-HEEP                                                       
** filename : dma.c                                                      
** version  : 1                                                            
** date     : 13/02/23                                                     
**                                                                         
***************************************************************************
**                                                                         
** Copyright (c) EPFL contributors.                                        
** All rights reserved.                                                    
**                                                                         
***************************************************************************
*/

/***************************************************************************/
/***************************************************************************/
/**
* @file   dma.c
* @date   13/02/23
* @brief  The Direct Memory Access (DMA) driver to set up and use the DMA 
* peripheral
*/

/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/

#include "dma.h"

/* To manage addresses. */
#include "mmio.h"

/* To manage interrupts. */
#include "fast_intr_ctrl.h"
#include "rv_plic.h"
#include "csr.h"
#include "stdasm.h"

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

 /**
 * Static DMA ASSERT.
 */
#define DMA_STATIC_ASSERT(expr, msg)// _Static_assert(!expr, msg);

/**
 * Returns the mask to enable/disable DMA interrupts.
 */
#define DMA_CSR_REG_MIE_MASK (( 1 << 19 ) | (1 << 11 ) ) // @ToDo Add definitions for this 19 and 11 

/**
 * Size of a register of 32 bits. 
 */
#define DMA_REGISTER_SIZE_BYTES sizeof(int)

/**
 * Mask to determine if an address is multiple of 4 (Word aligned).
 */
#define DMA_WORD_ALIGN_MASK 3

/**
 * Mask to determine if an address is multiple of 2 (Half Word aligned).
 */
#define DMA_HALF_WORD_ALIGN_MASK 1

/**
 * Selection offset to be used as index for when a register is written from 
 * the beginning. 
*/
#define DMA_SELECTION_OFFSET_START 0

/**
 * A small window size might result in loss of syncronism. If the processing 
 * of the window takes longer than the time it takes to the DMA to finish the 
 * next window, the application will not be able to cope. Although "how small 
 * is too small" is highly dependent on the length of the processing, this 
 * flag will be raised when the transaction and window size ratio is smaller 
 * than this arbitrarily chosen ratio as a mere reminder. 
 */
#define DMA_DEFAULT_TRANS_TO_WIND_SIZE_RATIO_THRESHOLD 4


/****************************************************************************/
/**                                                                        **/
/*                        TYPEDEFS AND STRUCTURES                           */
/**                                                                        **/
/****************************************************************************/

/**
 * Interrupts must be enabled in the INTERRUPT register of the DMA. 
 * Only one at a time. In case more than one is interrupt is to be triggered, 
 * at the same time (last byte of a transaction of size multiple of the window
 * size) only the lowest ID is triggered. 
 */ 
typedef enum
{
    INTR_EN_NONE        = 0x0, /*!< No interrupts should be triggered. */
    INTR_EN_TRANS_DONE  = 0x1, /*!< The TRANS_DONE interrupt is a fast 
    interrupt that is triggered once the whole transaction has finished. */ 
    INTR_EN_WINDOW_DONE = 0x2, /*!< The WINDOW_DONE interrupt is a PLIC 
    interrupt that is triggered every given number of bytes (set in the 
    transaction configuration as win_du). */
    INTR_EN__size,
} inter_en_t;

/****************************************************************************/
/**                                                                        **/
/*                      PROTOTYPES OF LOCAL FUNCTIONS                       */
/**                                                                        **/
/****************************************************************************/

/**
 * @brief Creates a target that can be used to perform transactions. 
 * @param p_tgt Pointer to the dma_target_t structure where configuration
 * should be allocated. The content of this pointer must be a static variable. 
 * @return A configuration flags mask. Each individual flag can be accessed with 
 * a bitwise AND ( ret & DMA_CONFIG_* ).
 */
dma_config_flags_t validate_target( dma_target_t *p_tgt );

/**
 * @brief Creates an environment where targets can be added. An environment 
 * is, for instance, a RAM block, or section of memory that can be used by 
 * the DMA. 
 * Properly defining an environment can prevent the DMA from accessing 
 * restricted memory regions. 
 * Targets for peripherals do not need an environment.
 * @param p_env Pointer to the dma_env_t structure where configuration 
 * should be allocated. The content of this pointer must be a static variable. 
 * @return A configuration flags mask. Each individual flag can be accessed 
 * with a bitwise AND ( ret & DMA_CONFIG_* ). It is not recommended to query 
 * the result from inside environment structure as an error could have 
 * appeared before the creation of the structure.
 */
dma_config_flags_t validate_environment( dma_env_t *p_env );

/**
 * @brief Gets how misaligned a pointer is, taking into account the data type 
 * size. 
 * @param p_ptr The source or destination pointer. 
 * @return How misaligned the pointer is, in bytes. 
 */
static inline uint8_t get_misalignment_b(   uint8_t         *p_ptr, 
                                            dma_data_type_t p_type );


/**
 * @brief Determines whether a given region will fit before the end of an 
 * environment.
 * @param p_start Pointer to the beginning of the region.
 * @param p_end Pointer to the last byte of the environment.
 * @param p_type The data type to be transferred.
 * @param p_size_du The number of data units to be transferred. Must be 
 * non-zero.
 * @param p_inc_du The size in data units of each increment. 
 * @retval 1 There is an outbound.
 * @retval 0 There is NOT an outbound.   
 */
static inline uint8_t is_region_outbound(   uint8_t  *p_start, 
                                            uint8_t  *p_end, 
                                            uint32_t p_type,  
                                            uint32_t p_size_du, 
                                            uint32_t p_inc_du );

/**
 * @brief Writes a given value into the specified register. Its operation 
 * mimics that of bitfield_field32_write(), but does not require the use of
 * a field structure, that is not always provided in the _regs.h file. 
 * @param p_val The value to be written.
 * @param p_offset The register's offset from the peripheral's base address
 *  where the target register is located.
 * @param p_mask The variable's mask to only modify its bits inside the whole
 * register.
 * @param p_sel The selection index (i.e. From which bit inside the register
 * the value is to be written). 
 */
static inline void write_register(  uint32_t p_val,   
                                    uint32_t p_offset, 
                                    uint32_t p_mask,  
                                    uint8_t  p_sel );


/**
 * @brief Analyzes a target to determine the size of its increment (in bytes). 
 * @param p_tgt A pointer to the target to analyze. 
 * @return The number of bytes of the increment.
 */ 
static inline uint32_t get_increment_b( dma_target_t * p_tgt );


/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED VARIABLES                             */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                            GLOBAL VARIABLES                              */
/**                                                                        **/
/****************************************************************************/

/**
 * Control Block (CB) of the DMA peripheral. 
 * Has variables and constant necessary/useful for its control. 
 */
static struct
{
    /**
    * Pointer to the transaction to be performed. 
    */
    dma_trans_t* trans;
    
    /**
    * Flag to lower as soon as a transaction is launched, and raised by the 
    * interrupt handler once it has finished. Only used when the end event is
    * set to INTR_WAIT.
    */
    uint8_t intrFlag;

    /**
     * memory mapped structure of a DMA.
     */
    dma *peri; 

}dma_cb;


/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

void dma_init( dma *peri )
{
    /* 
     * If a DMA peripheral was provided, use that one, otherwise use the
     * integrated one. 
     */
    dma_cb.peri = peri ? peri : dma_peri;

    /* Clear the loaded transaction */
    dma_cb.trans = NULL;
    /* Clear all values in the DMA registers. */
    dma_cb.peri->SRC_PTR       = 0;
    dma_cb.peri->DST_PTR       = 0;
    dma_cb.peri->SIZE          = 0;
    dma_cb.peri->PTR_INC       = 0;
    dma_cb.peri->SLOT          = 0;
    dma_cb.peri->DATA_TYPE     = 0;  
    dma_cb.peri->MODE          = 0;
    dma_cb.peri->WINDOW_SIZE   = 0;
    dma_cb.peri->INTERRUPT_EN  = 0;
}

dma_config_flags_t dma_validate_transaction(    dma_trans_t        *p_trans,  
                                                dma_en_realign_t   p_enRealign, 
                                                dma_perf_checks_t  p_check )
{
    /*
    * SANITY CHECKS
    */

    /* Data type is not necessary. If something is provided anyways it should
     be valid.*/
    DMA_STATIC_ASSERT( p_trans->type   < DMA_DATA_TYPE__size, 
                       "Data type not valid");
    /* Transaction mode should be a valid mode. */
    DMA_STATIC_ASSERT( p_trans->mode   < DMA_TRANS_MODE__size, 
                       "Transaction mode not valid");
    /* The end event should be a valid end event. */
    DMA_STATIC_ASSERT( p_trans->end    < DMA_TRANS_END__size, 
                       "End event not valid");
    /* The alignment permission should be a valid permission. */
    DMA_STATIC_ASSERT( p_enRealign     < DMA_ENABLE_REALIGN__size, 
                       "Alignment not valid");
    /* The checks request should be a valid request. */
    DMA_STATIC_ASSERT( p_check         < DMA_PERFORM_CHECKS__size, 
                       "Check request not valid");

    /*
     * CHECK IF TARGETS HAVE ERRORS
     */

    /* 
     * The transaction is NOT created if the targets include errors.
     * A successful target validation has to be done before loading it to the 
     * DMA.
     */
    uint8_t errorSrc = validate_target( p_trans->src );
    uint8_t errorDst = validate_target( p_trans->dst );

    /* 
     * If there are any errors or warnings in the valdiation of the targets, 
     * they are added to the transaction flags, adding the source/destination
     * identifying flags as well. 
     */
    p_trans->flags |= errorSrc ? (errorSrc | DMA_CONFIG_SRC ) : DMA_CONFIG_OK;
    p_trans->flags |= errorDst ? (errorDst | DMA_CONFIG_SRC ) : DMA_CONFIG_OK; 

    /* If a critical error was detected, no further action is performed. */
    if( p_trans->flags & DMA_CONFIG_CRITICAL_ERROR )
    {
        return p_trans->flags;
    }
    
    /*
     * CHECK IF THERE ARE TRIGGER INCONSISTENCIES 
     */

    /* 
     * The DMA can only handle one trigger at a time, therefore, if the two 
     * targets require a trigger, the transaction has to be discarded.
     * None of the two values can be taken by default because this 
     * inconsistency is probably a result of an error (likely wrong target 
     * selection).
     */
    if( p_check )
    {
        if(     p_trans->src->trig != DMA_TRIG_MEMORY
            &&  p_trans->dst->trig != DMA_TRIG_MEMORY ) 
        {
            p_trans->flags |= DMA_CONFIG_INCOMPATIBLE;
            p_trans->flags |= DMA_CONFIG_CRITICAL_ERROR;
            return p_trans->flags;
        }
    }

    /*
     * CHECK IF THERE IS A MODE INCONSISTENCY 
     */
    
    /* 
     * Memory to Memory in circular mode is not only of dubious usefulness, but also
     * risky. Depending on the MCU properties, the buffer size, the window size and
     * length of the ISR, the CPU could miss consecutive interrupts or even enter in
     * an irreversible loop state. 
     * This issue is less likely to happen with a properly set Memory-to-peripheral
     * configuration, so circular mode is allowed.
     */
    if( p_check )
    {
        if(    p_trans->src->trig == DMA_TRIG_MEMORY 
            && p_trans->dst->trig == DMA_TRIG_MEMORY 
            && p_trans->mode      == DMA_TRANS_MODE_CIRCULAR ) 
        {
            p_trans->flags |= DMA_CONFIG_INCOMPATIBLE;
            p_trans->flags |= DMA_CONFIG_CRITICAL_ERROR;
            return p_trans->flags;
        }
    } 

    /*
     * SET UP THE DEFAULT CONFIGURATIONS 
     */

    /* The flags are cleaned in case the structure was used before.*/
    p_trans->flags = DMA_CONFIG_OK;
    /* The copy size of the source (in data units -of the source-) is 
    transformed to bytes, to be used as default size.*/
    uint8_t dataSize_b = DMA_DATA_TYPE_2_SIZE(p_trans->src->type);
    p_trans->size_b = p_trans->src->size_du * dataSize_b;
    /* By default, the source defines the data type.*/
    p_trans->type = p_trans->src->type;
    /* 
     * By default, the transaction increment is set to 0 and, if required,
     * it will be changed to 1 (in which case both src and dst will have an
     * increment of 1 data unit).
     */
    p_trans->inc_b = 0;
    
    /*
     * CHECK IF THERE ARE MISALIGNMENTS 
     */

    if( p_check  )
    {
        /* 
         * The source and destination targets are analyzed. 
         * If the target is a peripheral (i.e. uses one of trigger slots)
         * then the misalignment is not checked.
         */
        uint8_t misalignment = 0;
        uint8_t dstMisalignment = 0;

        if( p_trans->src->trig == DMA_TRIG_MEMORY )
        {
            misalignment = get_misalignment_b( p_trans->src->ptr, p_trans->type );
        }
        
        if( p_trans->dst->trig == DMA_TRIG_MEMORY ) 
        {
            dstMisalignment = get_misalignment_b( p_trans->dst->ptr, p_trans->type );
        }

        p_trans->flags  |= ( misalignment ? DMA_CONFIG_SRC : DMA_CONFIG_OK ); 
        p_trans->flags  |= ( dstMisalignment ? DMA_CONFIG_DST : DMA_CONFIG_OK );

        /* Only the largest misalignment is preserved.*/
        if( misalignment < dstMisalignment )
        {
            misalignment = dstMisalignment;
        }

        if( misalignment != 0 )
        {
            /* 
             * Misalignment flags will only be stored in the transaction, as 
             * they are data type dependent, and could vary from transaction 
             * to transaction, even with the same pair of targets.
             */
            p_trans->flags |= DMA_CONFIG_MISALIGN;

            /* 
             * If a misalignment is detected and realignment is not allowed, 
             * an error is returned. No operation should be performed by the DMA.
             * No further operations are done to prevent corrupting information
             * that could be useful for debugging purposes. 
             */
            if( !p_enRealign)
            {
                return p_trans->flags |= DMA_CONFIG_CRITICAL_ERROR;
            }

            /*
             * CHECK IF THERE IS A DISCONTINUITY 
             */
 
            /*  
             * If there is a misalignment AND the source or destination 
             * arrangements are discontinuous, it is not possible to use the 
             * DMA. An error is returned.
             * A discontinuity in an arrangement is defined as the increment
             * being larger than the data type size. 
             * e.g. 
             * |AAAA|AAAA|BBBB|BBBB|      |____|AAAA|AAAA|____|  
             * 0    1    2    3           0    1    2    3          
             *    
             * |CCCC|CCCC|____|____| ==>  |____|BBBB|BBBB|____| 
             * 4    5    6    7           4    5    6    7     
             *    
             * |____|____|____|____|      |____|CCCC|CCCC|____| 
             * 8    9    10   11          8    9    10   11    
             * 
             * In this case the source arrangement has 16bit HALF WORDs, with
             * an increment of 1 data unit, so all data is continuous. 
             * The destination arrangement has also 16bit HALF WORDs, but 
             * misaligned and with 2 data units of increment. 
             * To copy a misaligned arrangement of HALF WORDs, the DMA should
             * use BYTEs. However, by using bytes it would not be able to 
             * write twice and skip twice, as it has a single skip increment. 
             * 
             * The misalignment and discontinuity can be found in the source 
             * and destination respectively and vice versa, and this 
             * limitation would still exist.  
             * 
             * The discontinuous flag is added (the misaligned one was 
             * already there), and it is turned into a critical error.'
             * 
             * No further operations are done to prevent corrupting
             * information that could be useful for debugging purposes.
             */
            if(    ( p_trans->src->inc_du > 1 ) 
                || ( p_trans->dst->inc_du > 1 ) )
            {
                p_trans->flags |= DMA_CONFIG_DISCONTINUOUS;
                p_trans->flags |= DMA_CONFIG_CRITICAL_ERROR;
                return p_trans->flags; 
            }

            /*
             * PERFORM THE REALIGNMENT 
             */
 
            /* 
             * If realignment is allowed and there are no discontinuities,
             * a more granular data type is used according to the detected 
             * misalignment in order to overcome it. 
             */
            p_trans->type += misalignment;
            /* 
             * Source and destination increment should now be of the size
             * of the data.
             * As increments are given in bytes, in both cases should be the
             * size of a data unit. 
             */
            p_trans->inc_b = DMA_DATA_TYPE_2_SIZE( p_trans->type );
            /* The copy size does not change, as it is already stored in bytes.*/
        }
    
        /*
         * CHECK IF SOURCE HAS SIZE 0
         */

        /*
         * No further operations are done to prevent corrupting information 
         * that could be useful for debugging purposes. 
         */
        if( p_trans->src->size_du == 0 ) 
        {
            p_trans->flags |= DMA_CONFIG_SRC;
            p_trans->flags |= DMA_CONFIG_CRITICAL_ERROR;
            return p_trans->flags; 
        }
        
        /*
         * CHECK IF THERE IS CROSS-OUTBOUND
         */

        /* 
         * As the source target does not know the constraints of the 
         * destination target, it is possible that the copied information
         * ends up beyond the bounds of the destination environment. 
         * This check is not performed if no destination environment was set.
         *
         * e.g. 
         * If 10 HALF WORDs are to be transferred with a HALF WORD in between
         * (increment of 2 data units: writes 2 bytes, skips 2 bytes, ...),
         * then ( 10 data units * 2 bytes ) + ( 9 data units * 2 bytes ) are
         * traveled for each = 38 bytes 
         * that need to be gone over in order to complete the transaction.
         * Having the transaction size in bytes, they need to be converted 
         * to data units:
         * size [data units] = size [bytes] / convertionRatio [bytes/data unit].
         * 
         * The transaction can be performed if the whole affected area can fit
         * inside the destination environment
         * (i.e. The start pointer + the 38 bytes -in this case-, is smaller
         * than the end pointer of the environment). 
         * 
         * No further operations are done to prevent corrupting information
         * that could be useful for debugging purposes.  
         */ 
        uint8_t isEnv = p_trans->dst->env;
        uint8_t isOutb = is_region_outbound(
                                    p_trans->dst->ptr, 
                                    p_trans->dst->env->end,
                                    p_trans->type,
                                    p_trans->src->size_du,
                                    p_trans->dst->inc_du );
        if( isEnv && isOutb )
        {
            p_trans->flags |= DMA_CONFIG_DST; 
            p_trans->flags |= DMA_CONFIG_OUTBOUNDS; 
            p_trans->flags |= DMA_CONFIG_CRITICAL_ERROR;
            return p_trans->flags;  
        }
        
        // @ToDo: It should also be checked that the destination is behind the 
        // source if there will be overlap. 
        // @ToDo: Consider if (when a destination target has no environment) 
        // the destination size should be used as limit. 

        /*
         * CHECK IF THE WINDOW SIZE IS ADEQUATE
         */

        /*
         * The window size cannot be larger than the transaction size. Although 
         * this would not cause any error, the transaction is rejected because 
         * it is likely a mistake. 
         */
        if( p_trans->win_du > p_trans->size_b )
        {
            p_trans->flags |= DMA_CONFIG_WINDOW_SIZE; 
            p_trans->flags |= DMA_CONFIG_CRITICAL_ERROR;
            return p_trans->flags; 
        }

        /*
         * If the ratio between the transaction size and the window size is too
         * large, the window size might be too small for the application to 
         * properly handle. The threshold is set in a weak implementation that 
         * can be overriden to adopt a custom value or set to return 0 to bypass
         * this check. 
         * The window size can also be set to 0 to not enable this trigger. 
         * This check only raises a warning, not a critical error as there is no 
         * certainty that an real error will occur.  
         */
        uint32_t threshold = dma_window_ratio_warning_threshold();
        uint32_t ratio = p_trans->size_b / p_trans->win_du;
        if(     p_trans->win_du 
            &&  threshold 
            &&  ( ratio > threshold) )
        {
            p_trans->flags |= DMA_CONFIG_WINDOW_SIZE; 
        }

    }   
        
    return p_trans->flags;
}

dma_config_flags_t dma_load_transaction( dma_trans_t *p_trans )
{
    /*
     * CHECK FOR CRITICAL ERRORS
     */

    /* 
     * The transaction is not allowed if it contain a critical error.
     * A successful transaction creation has to be done before loading it to
     * the DMA.
     */
    if( p_trans->flags & DMA_CONFIG_CRITICAL_ERROR )
    {   
        dma_cb.trans = NULL;
        return DMA_CONFIG_CRITICAL_ERROR;
    }
    
    /*
     * CHECK IF THERE IS A TRANSACTION RUNNING
     */

    /*
     * Modifying the DMA registers during the execution of a transaction can be 
     * dangerous. 
     * This is prevented by blocking any modification of the current transaction 
     * until it has ended. 
     * Transactions can still be validated in the meantime.
     */ 
    if( !dma_is_ready() )
    {
        return DMA_CONFIG_TRANS_OVERRIDE;
    }

    /* Save the current transaction */
    dma_cb.trans = p_trans; 
    
    /*
     * ENABLE/DISABLE INTERRUPTS
     */

    /* 
     * If the selected en event is polling, interrupts are disabled. 
     * Otherwise the mie.MEIE bit is set to one to enable machine-level
     * fast DMA interrupt.
     */
    dma_cb.peri->INTERRUPT_EN = INTR_EN_NONE;
    CSR_CLEAR_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK );

    if( dma_cb.trans->end != DMA_TRANS_END_POLLING )
    {
        /* Enable global interrupt for machine-level interrupts. */
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8 ); 
        /* @ToDo: What does this do? */
        CSR_SET_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK );

        dma_cb.peri->INTERRUPT_EN |= INTR_EN_TRANS_DONE;

        /* Only if a window is used should the window interrupt be set. */
        if( p_trans->win_du > 0 )
        {
            plic_Init();
            plic_irq_set_priority(DMA_WINDOW_INTR, 1);
            plic_irq_set_enabled(DMA_WINDOW_INTR, kPlicToggleEnabled);
            dma_cb.peri->INTERRUPT_EN |= INTR_EN_WINDOW_DONE;
        } 
    }

    /*
     * SET THE POINTERS
     */
    ////printf("src ptr - Wrote %08x @ ----\n", dma_cb.trans->src->ptr );
    //printf("dst ptr - Wrote %08x @ ----\n", dma_cb.trans->dst->ptr );
    dma_cb.peri->SRC_PTR = dma_cb.trans->src->ptr;

    if(dma_cb.trans->mode != DMA_TRANS_MODE_ADDRESS)
    {
        /*
            Write to the destination pointers only if we are not in address mode,
            otherwise the destination address is read in a separate port in parallel with the data
            from the address port
        */
        dma_cb.peri->DST_PTR = dma_cb.trans->dst->ptr;
     }
     else
     {
        dma_cb.peri->ADDR_PTR = dma_cb.trans->src_addr->ptr;
     }

    if(dma_cb.trans->mode != DMA_TRANS_MODE_ADDRESS)
    {
        /*
            Write to the destination pointers only if we are not in address mode,
            otherwise the destination address is read in a separate port in parallel with the data
            from the address port
        */
        dma_cb.peri->DST_PTR = dma_cb.trans->dst->ptr;
     }
     else
     {
        dma_cb.peri->ADDR_PTR = dma_cb.trans->src_addr->ptr;
     }

    /*
     * SET THE INCREMENTS
     */

    /* 
     * The increments might have been changed (vs. the original value of 
     * the target) due to misalignment issues. If they have, use the changed
     * values, otherwise, use the target-specific ones.
     * Other reason to overwrite the target increment is if a trigger is used.
     * In that case, a increment of 0 is necessary.
     * In case of DMA Address mode transaction, the dst pointer is ignored
     * as the values read from the second port are instead used.
     */

    //printf("src inc - ");
    write_register(  get_increment_b( dma_cb.trans->src ), 
                    DMA_PTR_INC_REG_OFFSET, 
                    DMA_PTR_INC_SRC_PTR_INC_MASK,
                    DMA_PTR_INC_SRC_PTR_INC_OFFSET );
   
   

    if(dma_cb.trans->mode != DMA_TRANS_MODE_ADDRESS)
    {
        //printf("dst inc - ");
        write_register(  get_increment_b( dma_cb.trans->dst ),
                        DMA_PTR_INC_REG_OFFSET,
                        DMA_PTR_INC_DST_PTR_INC_MASK,
                        DMA_PTR_INC_DST_PTR_INC_OFFSET );
    }


    /*
     * SET THE OPERATION MODE AND WINDOW SIZE
     */

    //printf("mode    - Wrote %08x @ ----\n", dma_cb.trans->mode );
    dma_cb.peri->MODE = dma_cb.trans->mode;
    /* The window size is set to the transaction size if it was set to 0 in
    order to disable the functionality (it will never be triggered). */
    
    //printf("win siz - Wrote %08x @ ----\n", dma_cb.trans->win_du ? dma_cb.trans->win_du : dma_cb.trans->size_b );

    dma_cb.peri->WINDOW_SIZE =   dma_cb.trans->win_du 
                            ? dma_cb.trans->win_du
                            : dma_cb.trans->size_b;
    
    /*
     * SET TRIGGER SLOTS AND DATA TYPE
     */    
    //printf("src tri - ");
    write_register(  dma_cb.trans->src->trig, 
                    DMA_SLOT_REG_OFFSET, 
                    DMA_SLOT_RX_TRIGGER_SLOT_MASK,
                    DMA_SLOT_RX_TRIGGER_SLOT_OFFSET );

    //printf("dst tri - ");
    write_register(  dma_cb.trans->dst->trig, 
                    DMA_SLOT_REG_OFFSET, 
                    DMA_SLOT_TX_TRIGGER_SLOT_MASK,
                    DMA_SLOT_TX_TRIGGER_SLOT_OFFSET );

    //printf("dat typ - ");
    write_register(  dma_cb.trans->type, 
                    DMA_DATA_TYPE_REG_OFFSET, 
                    DMA_DATA_TYPE_DATA_TYPE_MASK, 
                    DMA_SELECTION_OFFSET_START );
    
    return DMA_CONFIG_OK;
}

dma_config_flags_t dma_launch( dma_trans_t *p_trans )
{
    /*
     * Make sure that the loaded transaction is the intended transaction. 
     * If the loaded trans was NULL'd, then this the transaction is never 
     * launched.
     */
    if(     ( p_trans == NULL ) 
        ||  ( dma_cb.trans != p_trans ) ) // @ToDo: Check per-element.
    {
        return DMA_CONFIG_CRITICAL_ERROR;
    }

    /*
     * CHECK IF THERE IS A TRANSACTION RUNNING
     */

    /*
     * Modifying the DMA registers during the execution of a transaction can be 
     * dangerous. 
     * This is prevented by blocking any modification of the current transaction 
     * until it has ended. 
     * Transactions can still be validated in the meantime.
     */ 
    if( !dma_is_ready() )
    {
        return DMA_CONFIG_TRANS_OVERRIDE;
    }

    /*
     * This has to be done prior to writing the register because otherwise
     * the interrupt could arrive before it is lowered.
     */
    dma_cb.intrFlag = 0;

    /* Load the size and start the transaction. */
    //printf("size    - Wrote %08x @ ----\n", dma_cb.trans->size_b );
    dma_cb.peri->SIZE = dma_cb.trans->size_b;

    /* 
     * If the end event was set to wait for the interrupt, the dma_launch
     * will not return until the interrupt arrives. 
     */
    while(    p_trans->end == DMA_TRANS_END_INTR_WAIT 
          && ( dma_cb.intrFlag != 0 ) ) { // @ToDo: add a label for this 0
        wait_for_interrupt();
    }

    return DMA_CONFIG_OK;
}


uint32_t dma_is_ready(void)
{    
    /* The transaction READY bit is read from the status register*/   
    uint32_t ret = ( dma_cb.peri->STATUS & (1<<DMA_STATUS_READY_BIT) );
    return ret;
}
/* @ToDo: Reconsider this decision.
 * In case a return wants to be forced in case of an error, there are 2 
 * alternatives: 
 *    1) Consider any value != 0 to be a valid 1 using a LOGIC AND: 
 *  return ( 1 && dma_cb.peri->DONE );
 *    2) Consider only the LSB == 1 to be a valid 1 using a BITWISE AND. 
 *  return ( 1 &  dma_cb.peri->DONE );
 * This would be fixed if the DONE register was a 1 bit field. 
 */   


uint32_t dma_get_window_count()
{
    return dma_cb.peri->WINDOW_COUNT;
}


void dma_stop_circular()
{
    /*
     * The DMA finishes the current transaction before and does not start
     * a new one.
     */
    dma_cb.peri->MODE = DMA_TRANS_MODE_SINGLE;
}


__attribute__((weak, optimize("O0"))) void dma_intr_handler_trans_done()
{
    /* 
     * The DMA transaction has finished!
     * This is a weak implementation.
     * Create your own function called 
     * void dma_intr_handler_trans_done() 
     * to override this one.  
     */
}

__attribute__((weak, optimize("O0"))) void dma_intr_handler_window_done()
{
    /* 
     * The DMA has copied another window.
     * This is a weak implementation.
     * Create your own function called 
     * void dma_intr_handler_window_done() 
     * to override this one.  
     */
}

__attribute__((weak, optimize("O0"))) uint8_t dma_window_ratio_warning_threshold()
{
    /* 
     * This is a weak implementation.
     * Create your own function called 
     * void dma_window_ratio_warning_threshold() 
     * to override this one.  
     * Make it return 0 to disable this warning. 
     */
    return DMA_DEFAULT_TRANS_TO_WIND_SIZE_RATIO_THRESHOLD;
}
 
/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

dma_config_flags_t validate_target( dma_target_t *p_tgt )
{
    /* Flags variable to pass encountered errors. */
    dma_config_flags_t flags;
    /*
     * SANITY CHECKS
     */

    /* Increment can be 0 when a trigger is used. */
    DMA_STATIC_ASSERT( p_tgt->inc_du   >= 0 , "Increment not valid"); 
    /* The size could be 0 if the target is only going to be used as a 
    destination. */
    DMA_STATIC_ASSERT( p_tgt->size_du  >=  0 , "Size not valid"); 
    /* The data type must be a valid type */
    DMA_STATIC_ASSERT( p_tgt->type     < DMA_DATA_TYPE__size , "Type not valid");
    /* The trigger must be among the valid trigger values. */
    DMA_STATIC_ASSERT( p_tgt->trig     < DMA_TRIG__size , "Trigger not valid");
    
    /*
     * INTEGRITY CHECKS
     */
    
    /* 
     * Check if the copy will always be inside the target's environments
     * boundaries.
     * This check is only performed if an environment was set.
     */
    if( p_tgt->env != NULL )
    {   
        /* Check if the environment was properly formed.*/
        flags |= validate_environment( p_tgt->env );

        /* 
         * Check if the target selected size goes beyond the boundaries of
         * the environment. 
         * This is only analyzed if a size was defined. 
         */
        if( p_tgt->size_du != 0 )
        {
            uint8_t isOutb = is_region_outbound(  p_tgt->ptr,
                                          p_tgt->env->end, 
                                          p_tgt->type, 
                                          p_tgt->size_du, 
                                          p_tgt->inc_du );
            if( isOutb )
            {
                flags |= DMA_CONFIG_OUTBOUNDS;
            }
        }

        /* Check if the target starts before the environment starts. */
        uint8_t beforeEnv = ( p_tgt->ptr < p_tgt->env->start );
        /* Check if the target starts after the environment ends. */
        uint8_t afterEnv  = ( p_tgt->ptr > p_tgt->env->end );
        if( beforeEnv || afterEnv )
        {
            flags |= DMA_CONFIG_OUTBOUNDS;
        }
    }

    /* 
    * If there is a trigger, there should not be environments
    * nor increments (or it is an incompatible peripheral).
    * Otherwise, an increment is needed (or it is an incompatible
    * memory region).
    */

    if( p_tgt->trig == DMA_TRIG_MEMORY ){ /* If it is a memory region. */
        /* It should have an increment. */
        if( ( p_tgt->inc_du == 0 ) ){ 
            flags |= DMA_CONFIG_INCOMPATIBLE;
        }
    }
    else /* If it is a peripheral. */
    {   
        /* It should not have neither an environment nor an increment. */
        if( (     (p_tgt->env != NULL) 
              ||  ( p_tgt->inc_du != 0 ) ) )
        {
            flags |= DMA_CONFIG_INCOMPATIBLE;
        }
    }

    /*
     * This is returned so this function can be called as: 
     * if( validate_target == DMA_CONFIG_OK ){ go ahead } 
     * or if( validate_target() ){ check for errors }
     */
    return flags; 
}

dma_config_flags_t validate_environment( dma_env_t *p_env )
{
    /*
     * SANITY CHECKS
     */
    if( (uint8_t*)p_env->end < (uint8_t*)p_env->start ) 
    {
        return DMA_CONFIG_INCOMPATIBLE;
    }    
    return DMA_CONFIG_OK;
}
/* @ToDo: Prevent validation of targets whose environment was not validated. */

static inline uint8_t get_misalignment_b(  uint8_t         *p_ptr, 
                                      dma_data_type_t p_type )
{
    /*
     * Note: These checks only makes sense when the target is memory. This is 
     * not performed when the target is a peripheral (i.e. uses a trigger). 
     * Check for word alignment:
     * The 2 LSBs of the data type must coincide with the 2 LSBs of the SRC 
     * pointer.
     * This guarantees word alignment. 
     * |____|____|____|____|____|____|____|____|    Memory address 0x*******y
     * 0    1    2    3    4    5    6    7     = y (In bytes)
     *  Byte words can start in any of these positions
     *  Half Words can only start on 0, 2, 4 or 6
     *  Words can only start on 0 or 4
     *  For example, if there was a Word starting on address ended in 2:
     * |____|____|\\\\|\\\\|\\\\|\\\\|____|____|
     * 0    1    2    3    4    5    6    7    
     * The DMA could only grab bytes 0-3 and 4-7, so it CANNOT copy into the 
     * destination pointer (x) the desired Word as follows:
     * |\\\\|\\\\|\\\\|\\\\|
     * x   x+1  x+2  x+3   
     * 
     * To overcome this, the ALLOW REALIGN flag is available in the DMA 
     * control block. 
     * If the user set the ALLOW REALIGN flag, WORD reading from misaligned 
     * pointers will be converted to two HALF WORD readings with a HALF WORD
     * increment on both source and destination.  
     * 
     * HALF WORD misalignment is solved through the same method using two WORD
     * readings. 
     * 
     */  
    
    uint8_t misalignment;

    /* 
     * If the data type is WORD and the two LSBs of pointer are not 00,
     * there is a misalignment.
     */
    
    if(  p_type == DMA_DATA_TYPE_WORD )
    {
        if( ( (uint32_t)p_ptr & DMA_WORD_ALIGN_MASK )  != 0 )
        {
            misalignment++;
        }
    }
    
    /* 
     * If the data type is WORD or HALF WORD and the LSB of pointer 
     * is not 0, there is a misalignment.
     * The inequality is of special importance because WORDs stored in odd
     * pointers need to turn into BYTE as well.
     */
    if( p_type <= DMA_DATA_TYPE_HALF_WORD )
    {
        if( ( (uint32_t)p_ptr & DMA_HALF_WORD_ALIGN_MASK ) != 0 )
        {
            misalignment++;

        }
    }

    /* 
     * These two operations will end up with: 
     * misalignment == 0 if no realignment is needed.
     * misalignment == 1 if realignment is needed, but switching to half 
     * the word size would fix it
     * misalignment == 2 if a WORD is to be read from an odd pointer, so 
     * BYTE data type is needed necessarily. 
     */
    return misalignment;
}

static inline uint8_t is_region_outbound( uint8_t  *p_start, 
                                  uint8_t  *p_end, 
                                  uint32_t p_type, 
                                  uint32_t p_size_du, 
                                  uint32_t p_inc_du )
{
  /* 000 = A data unit to be copied
   * xxx = A data unit to be skipped
   * 
   * v The start               /------------\ The size of each increment
   * |OOOO|xxxx|xxxx|OOOO|xxxx|xxxx| . . . |OOOO|xxxx|xxxx|OOOO|xxxx|xxxx| 
   *  \--/ The size of a type    
   *  \------------------- Each increment n-1 times ------/
   *                              + 1 word (w/o increment) \--/  
   *  \------ All the affected region (rangeSize) ------------/ 
   *                                   The last affected byte ^ 
   * 
   * If the environment ends before the last affected byte, then there is
   * outbound writing and the function returns 1.  
   */
    uint32_t affectedUnits      = ( p_size_du - 1 ) * p_inc_du + 1;
    uint32_t rangeSize          = DMA_DATA_TYPE_2_SIZE(p_type) * affectedUnits;
    uint32_t lasByteInsideRange = p_start + rangeSize -1;
    return ( p_end < lasByteInsideRange );
    // Size is be guaranteed to be non-zero before calling this function.  
}

/* @ToDo: Consider changing the "mask" parameter for a bitfield definition 
(see dma_regs.h) */
static inline void write_register( uint32_t  p_val, 
                                  uint32_t  p_offset, 
                                  uint32_t  p_mask,
                                  uint8_t   p_sel )
{
    /* 
     * The index is computed to avoid needing to access the structure
     * as a structure. 
     */
    uint8_t index = p_offset / DMA_REGISTER_SIZE_BYTES;
    /* 
     * An intermediate variable "value" is used to prevent writing twice into 
     * the register.
     */
    uint32_t value  =  (( uint32_t * ) dma_cb.peri ) [ index ];
    value           &= ~( p_mask << p_sel );
    value           |= (p_val & p_mask) << p_sel;
    (( uint32_t * ) dma_cb.peri ) [ index ] = value; 

    //printf("Wrote %08x @ %04x\n", value, index );

// @ToDo: mmio_region_write32(dma->base_addr, (ptrdiff_t)(DMA_SLOT_REG_OFFSET), (tx_slot_mask << DMA_SLOT_TX_TRIGGER_SLOT_OFFSET) + rx_slot_mask)

}

static inline uint32_t get_increment_b( dma_target_t * p_tgt )
{
    uint32_t inc_b = 0;
    /* If the target uses a trigger, the increment remains 0. */
    if(  p_tgt->trig  == DMA_TRIG_MEMORY ) 
    {   
        /*
         * If the transaction increment has been overriden (due to 
         * misalignments), then that value is used (it's always set to 1).
         */
        inc_b = dma_cb.trans->inc_b;

        /*
        * Otherwise, the target-specific increment is used transformed into
        * bytes).
        */
        if( inc_b == 0 )
        {
            uint8_t dataSize_b = DMA_DATA_TYPE_2_SIZE( dma_cb.trans->type );
            inc_b = ( p_tgt->inc_du * dataSize_b );
        }
    }
    return inc_b;
}


/** 
 * This is a non-weak implementation of the function declared in fast_intr_ctrl.c
 */
void fic_irq_dma(void)
{
    /* The flag is raised so the waiting loop can be broken.*/
    dma_cb.intrFlag = 1;

    /*
     * Call the weak implementation provided in this module, 
     * or the non-weak implementation.
     */ 
    dma_intr_handler_trans_done();
}

/** 
 * This is a non-weak implementation of the function declared in rv_plic.c
 */
void handler_irq_dma(uint32_t id) 
{
    /*
     * Call the weak implementation provided in this module, 
     * or the non-weak implementation.
     */ 
    dma_intr_handler_window_done();
}

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/