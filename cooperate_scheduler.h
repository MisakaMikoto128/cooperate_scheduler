/**
 * @file cooperate_scheduler.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-14
 * 
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 * 
 */

#ifndef cooperate_scheduler_H
#define cooperate_scheduler_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>
#include "sc_list.h"

typedef void (*Function_t)(void *arg);

typedef struct tagFunctional
{
    Function_t fun;
    void *arg;
} Functional_t;

#define cooperate_scheduler_EXE_TIMES_INF UINT_MAX

typedef struct tagcooperate_schedulerTask
{
    /*��һ��ִ�е�CPUʱ�����ÿ��ִ�е�ʱ����ߵ�һ�����������ʱ����¡�
    ����ǰCPU tick >= last_exe_tick+exe_timesʱִ�У�֮�����last_exe_tick��
    */
    uint32_t last_exe_tick;//��һ�ε��Ȳ���ִ���˵�ʱ��
    uint32_t exe_times; //��Ҫִ�ж��ٴΣ�
    uint32_t exe_cnt;   //ִ���˶��ٴ�
    uint32_t period;    //ִ�����ڣ���λΪCPU tick�����ڣ��������0����������൱����ѭ���ˡ�
    uint32_t delay_before_first_exe;//��ע���һ������ִ�е��ȵ��ӳ�
    uint32_t register_tick;//ע��ʱ��
    Functional_t fun;   //ִ�еķ���
    struct sc_list next;


    int32_t _exe_tick_error;//ִ��ʱ�����
    uint32_t _elapsed_tick_since_last_exe;//�����ϴ�ִ����ȥ�˶����¼�
} cooperate_schedulerTask_t;

/**
 * @brief cooperate_scheduler��ʼ����
 *
 */
void cooperate_scheduler_init();

/**
 * @brief cooperate_scheduler��������
 * ֻ�б�֤cooperate_scheduler_handlerִ��Ƶ�ʴ���cooperate_scheduler��ʱ���ļ�ʱ�ֱ��ʵ�Ƶ�ʲ��ܱ�֤ʱ�����׼ȷ�Ķ�ʱ������ȡ�
 */
void cooperate_scheduler_handler();

/**
 * @brief ������ڵ�ע�ᵽcooperate_scheduler�С�
 *
 * @param sche_node
 * @retval true ע��ɹ���false ע��ʧ�ܡ�
 */
bool cooperate_scheduler_register(cooperate_schedulerTask_t *task);


/**
 * @brief �����Ƿ���ע���״̬��
 *
 * @param sche_node
 * @retval true ��Ȼ��ע��ģ�false δע�ᡣ
 */
bool cooperate_scheduler_is_task_registered(cooperate_schedulerTask_t *task);

/**
 * @brief ȡ���Ѿ�ע�ᵽcooperate_scheduler�е�����
 *
 * @param task
 * @return true �������ע�������������ԭ����ȡ��ע��ʧ�ܡ�
 * @return false ����δע�������������ԭ��ȡ��ע��ɹ���
 */
bool cooperate_scheduler_unregister(cooperate_schedulerTask_t *task);

/**
 * @brief ��������ڵ��Ƶ�ʡ�
 * 
 * @param task 
 * @param freq 1-1000Hz
 * @return None 
 */
void cooperate_scheduler_set_freq(cooperate_schedulerTask_t *task,int freq);

typedef uint32_t Period_t;
#define MAX_PERIOD_ID 10 //��������ID�ţ���0��ʼ������

/**
 * @brief ��ѯ�Ƿ�����Ҫ�����ڡ���������и��ٲ�ѯ������ж����ڵ��ˣ��ͻ�
 * ����true�����򷵻�false,���ҵ����ڵ���֮������last_exe_tick����֤ÿ����ֻ����
 * �Ͻ��Ϊ��һ�Ρ���������ѭ���з���Ĺ���������ִ�еĴ���Ρ�
 *
 * ����һ��Period_t�����һ��ִ��ʱ���ʱ�����period_id��ʶ��
 * @param period_id ����id��ȫ��Ψһ��
 * @param period ���ڡ�
 * @return true ���ڵ���
 * @return false ����δ����
 */
bool period_query(uint8_t period_id, uint32_t period);
/**
 * @brief ͬperiod_query_user��ֻ��ʱ���¼��һ��uint32_t*ָ��ı����С�
 *
 * @param period_recorder ��¼����ʱ��ı�����ָ�롣
 * @param period ���ڡ�
 * @return true ���ڵ���
 * @return false ����δ����
 */
bool period_query_user(uint32_t* period_recorder, uint32_t period);
#endif //! cooperate_scheduler_H