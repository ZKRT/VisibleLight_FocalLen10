#ifndef _CAMERA_H
#define _CAMERA_H

#include "sys.h"

#define _PWM_2000	2000
#define _PWM_1000	1000

#define _TIM_SBUS		 20
#define _TIM_PAIZHAO 1000
#define _TIM_RETURN  500

//PWM1_T3C2����ӦPB5
//PWM2_T3C1����ӦPB4
//PWM3_T2C1����ӦPA15
//PWM4_T3C4����ӦPB1
//PWM5_T3C3����ӦPB0
//PWM6_T2C4����ӦPA3
//PWM7_T2C3����ӦPA2
//PWM8_T2C2����ӦPA1

#define PWM_HANGXAING(value)      TIM_SetCompare2(TIM3, value);
#define PWM_MODE(value) 		      TIM_SetCompare1(TIM3, value);
#define PWM_FUYANG(value)         TIM_SetCompare1(TIM2, value);
#define PWM_JIAOJU(value)         TIM_SetCompare4(TIM3, value);
#define PWM_DONGZUO_MODE(value)   TIM_SetCompare3(TIM2, value);  //���պ�¼��ģʽ�л�
#define PWM_DONGZUO(value)        TIM_SetCompare2(TIM2, value);

#define GET_JIAOJU                TIM_GetCapture4(TIM3)
#define GET_DONGZUO               TIM_GetCapture2(TIM2)
#define GET_DONGZUO_MODE          TIM_GetCapture3(TIM2)

//¼�� ���� macro
#define DONGZUO_MODE_REC					0
#define DONGZUO_MODE_PIC					1

//��ʼ¼�� ֹͣ¼�� macro
#define START_RECORD              0
#define STOP_RECORD               1
#define RECORD_NONE               2


extern volatile uint32_t paizhao_count;
extern volatile uint32_t return_count;

extern uint16_t last_paizhao;
extern uint16_t last_shexiang;
extern uint8_t last_jiaoju_re;
extern uint8_t last_jiaoju_in;
extern uint8_t last_three_paizhao;

extern uint16_t pwm_hangxiang;	//ͨ��1������
extern uint16_t pwm_fuyang;			//ͨ��2������
extern uint16_t pwm_henggun;		//ͨ��3�����
extern uint16_t pwm_paizhao;		//ͨ��5������
extern uint16_t pwm_shexiang;		//ͨ��6������
extern uint8_t pwm_jiaoju_re;		//ͨ��7�������С
extern uint8_t pwm_jiaoju_in;		//ͨ��7��������		
extern uint16_t pwm_mode;				//ͨ��8��ģʽ�л�
extern uint8_t pwm_three_paizhao;//�ֽ�28��������
extern uint8_t pwm_init_flag; 	//�ֽ�29����ʼ����ʶ


uint8_t sub_camera_zkrt_recv_decode_and_zkrt_encode_ack(void);
void action_hangxiang(void);
void action_fuyang(void);
void action_henggun(void);
void action_paizhao(void);
void action_shexiang(void);
void action_jiaoju(void);
void action_mode(void);
void action_dongzuo_mode(uint8_t mode);


#endif

