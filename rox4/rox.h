#ifndef ROX_H
#define ROX_H

#include "rule.h" //Get def of Rule
#include <linux/ioctl.h>
#include <linux/param.h>


#define MAJOR_NUM 100   //major number of device


/*
 * _IOR means that we're creating an ioctl command
 * number for passing information from a user process
 * to the kernel module.
 *
 * The first arguments, MAJOR_NUM, is the major device 
 * number we're using.
 *
 * The second argument is the number of the command 
 * (there could be several with different meanings).
 *
 * The third argument is the type we want to get from 
 * the process to the kernel.
 */

//Insert a new rule to rules
#define IOCTL_ROX_INSERT_RULE _IOR(MAJOR_NUM, 0, struct Rule*) 

//Delete a rule from rules
#define IOCTL_ROX_DELETE_RULE _IOR(MAJOR_NUM, 1, struct Rule*) 

//Update an old rule 
#define IOCTL_ROX_UPDATE_RULE _IOR(MAJOR_NUM, 2, struct Rule*) 

//Print current rule table
#define IOCTL_ROX_PRINT_RULE  _IOR(MAJOR_NUM, 3, struct Rule*)

#define DEVICE_NAME "rox"
#define DEVICE_FILE_NAME "/dev/rox"
#define SUCCESS 0

#endif