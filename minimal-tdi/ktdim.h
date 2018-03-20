#ifndef _TIMEDOCTOR_H
#define _TIMEDOCTOR_H

/******************************************************************************/
/**
 * \brief These types describe the command to log
 *
 * \sa    TDI_COMMAND
 */
/******************************************************************************/
#define TDI_DELETE  4
#define TDI_CREATE  3
#define TDI_GENERIC 2
#define TDI_START   1
#define TDI_STOP    0

/******************************************************************************/
/**
 * \brief  These types describe the kind of object to log
 *
 * \sa     TDI_COMMAND
 */
/******************************************************************************/
#define TDI_TASK   0
#define TDI_ISR    1
#define TDI_SEM    2
#define TDI_QUEUE  3
#define TDI_EVENT  4
#define TDI_AGENT  8

#define TDI_CR     2
#define TDI_TRACE  3

#define TIMEDOCTOR_INFO_DATASIZE (3)

#define TIMEDOCTOR_IOCTL_RESET           _IO( 'T', 0)
#define TIMEDOCTOR_IOCTL_START           _IO( 'T', 1)
#define TIMEDOCTOR_IOCTL_STOP            _IO( 'T', 2)
#define TIMEDOCTOR_IOCTL_GET_ENTRIES     _IO( 'T', 3)
#define TIMEDOCTOR_IOCTL_GET_MAX_ENTRIES _IO( 'T', 4)
#define TIMEDOCTOR_IOCTL_INFO            _IO( 'T', 5)

/******************************************************************************/
/**
 * \brief  This macro is used to build the first argument of the main log
 *         API in a way that is compliant with time doctor
 *
 * \sa     TDI_COMMAND
 */
/******************************************************************************/
#define TDI_COMMAND(cmd, type) \
    (cmd << 24 | type << 16)

/******************************************************************************/
/**
 * \brief  This macro is used to convert a 4 char array into a 32 bits word.
 *         It is typically used to convert task names into a 32 bit word.
 *
 * \sa     TDI_COMMAND
 */
/******************************************************************************/
#define CHAR_ARRAY_TO_U32(array) \
    ((unsigned int)((array[0]<<24)|(array[1]<<16)|(array[2]<<8)|array[3]))

#define CHAR2_ARRAY_TO_U32(array) \
    ((unsigned int)((array[4]<<24)|(array[5]<<16)|(array[6]<<8)|array[7]))

/******************************************************************************/
/**
 * \brief  This macro is used to log the creation of a task.
 *
 * \sa     
 */
/******************************************************************************/
#define timeDoctor_task_create(Tid, Name) \
    timeDoctor_Info(TDI_COMMAND(TDI_CREATE, TDI_TASK) | Tid, CHAR_ARRAY_TO_U32(Name), CHAR2_ARRAY_TO_U32(Name))

/******************************************************************************/
/**
 * \brief  This macro is used to log the deletion of a task.
 *
 * \sa     
 */
/******************************************************************************/
#define timeDoctor_task_delete(Tid, Name) \
    timeDoctor_Info(TDI_COMMAND(TDI_DELETE, TDI_TASK) | Tid, Name, 0)

/******************************************************************************/
/**
 * \brief  This macro is used to log the switch of tasks
 *
 * \sa     
 */
/******************************************************************************/
#define timeDoctor_task_switch( NewTid, NewName, \
                            OldTid, OldName) \
    timeDoctor_Info(TDI_COMMAND(TDI_STOP, TDI_TASK) | OldTid, CHAR_ARRAY_TO_U32(OldName), CHAR2_ARRAY_TO_U32(OldName)); \
    timeDoctor_Info(TDI_COMMAND(TDI_START, TDI_TASK) | NewTid, CHAR_ARRAY_TO_U32(NewName), CHAR2_ARRAY_TO_U32(NewName))

/******************************************************************************/
/**
 * \brief  This macro is used to log the start of an ISR
 *
 * \sa     
 */
/******************************************************************************/
#define timeDoctor_interrupt_enter(isr_id) \
    timeDoctor_Info(TDI_COMMAND(TDI_START, TDI_ISR) | isr_id, 0, 0)

/******************************************************************************/
/**
 * \brief  This macro is used to log the exit of an ISR
 *
 * \sa     
 */
/******************************************************************************/
#define timeDoctor_interrupt_exit(isr_id) \
    timeDoctor_Info(TDI_COMMAND(TDI_STOP, TDI_ISR) | isr_id, 0, 0)

/******************************************************************************/
/** \brief  This function records an event in the event buffer. An event is 
 *          composed of a 32 bit time stamp and 3 data of 32 bits length.
 *
 *  \param  data1 : data to log
 *  \param  data2 : data to log
 *  \param  data3 : data to log
 *
 *  \return None
 *
 *  \sa     
 */
/******************************************************************************/
void
timeDoctor_Info(
        unsigned int data1,
        unsigned int data2,
        unsigned int data3);

/******************************************************************************/
/** \brief  This function changes the level of trace.
 *
 *  \param  level : 0 -> trace off
 *                  1 -> trace on
 *
 *  \return None
 *
 *  \sa     
 */
/******************************************************************************/
void
timeDoctor_SetLevel(
        int level);

/******************************************************************************/
/** \brief  This function reset the event buffer.
 *
 *  \return None
 *
 *  \sa     
 */
/******************************************************************************/
void
timeDoctor_Reset(
        void);

#endif /* _TIMEDOCTOR_H */
