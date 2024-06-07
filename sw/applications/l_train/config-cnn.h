/*!***********************************************************************************
 * @file     config-cnn.h
 * @author 	 Linus Crugnola	<linus.crugnola@epfl.ch>
 * @date     07.06.2024
 *
 * Description:
 *  This file contains general configuration for the CNN model.
 *
 *************************************************************************************/

#ifndef CONFIG_CNN_H
#define CONFIG_CNN_H

/**
 * @def DYN_ALLOCATION
 * @brief define this macro to enable dynamic allocation of the model
 */
// #define DYN_ALLOCATION

/**
 * @def TRACK_LOSS
 * @brief define this macro to enable tracking of the loss
 */
#define TRACK_LOSS

/**
 * @def LOG_GRADIENTS
 * @brief define this macro to enable logging of the gradients
 */
// #define LOG_GRADIENTS

#endif // CONFIG_CNN_H