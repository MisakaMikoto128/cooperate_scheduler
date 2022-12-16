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
#include "cpu_time.h"

/* Config CPU Tick here */
#define scheduler_get_cpu_tick() (getCurrentMilliSecTimestamp() - 1671163704392ULL)

typedef bool (*BoolFunction_t)(void *arg);

typedef struct tagBoolFunctional_t
{
    BoolFunction_t fun;
    void *arg;
} BoolFunctional_t;

typedef struct tagFunctionalListNode_t
{
    char *name;
    /*��һ��ִ�е�CPUʱ�����ÿ��ִ�е�ʱ����ߵ�һ�����������ʱ����¡�
    ����ǰCPU tick >= last_exe_tick+exe_timesʱִ�У�֮�����last_exe_tick��
    */
    uint32_t last_exe_tick; // ��һ�ε��Ȳ���ִ�гɹ��˵�ʱ�̻��ߵ��������һ�ε�ִ��ʱ��
    uint32_t exe_times;     // ��Ҫִ�ж��ٴΣ�
    uint32_t exe_cnt;       // ִ���˶��ٴ�
    uint32_t exe_fail_cnt;  // ִ��ʧ���˶��ٴ�
    uint32_t period;        // ִ�����ڣ���λΪCPU tick�����ڣ��������0����������൱����ѭ���ˡ�

    uint32_t delay_before_first_exe;       // ��ע���һ������ִ�е��ȵ��ӳ�
    uint32_t register_tick;                // ע��ʱ��
    BoolFunctional_t fun;                      // ִ�еķ���
    int32_t _exe_tick_error;               // ִ��ʱ�����
    uint32_t _elapsed_tick_since_last_exe; // �����ϴ�ִ����ȥ�˶����¼�

    struct sc_list next;
} TaskNode_t;

typedef struct tagCooperativeGroup_t
{
    // ÿ��group��ÿ����������Сִ��ʱ����,���һ����ռ�õ���Դ������ռ�õģ���ô��
    // �����min_period��ѯ��ǰ����ֱ����Դ���ͷţ��������Ż�ִ�гɹ�(��ȡ����Դ)��
    // ��λ��tick,��Χ >= 0����������������С��Դռ��ʱ��ָ����
    // �������е����񶼻�ռ��ͬһ��ȫ����Դ��ÿ������ռ�õ�ʱ����ܲ�һ������������
    // һ����С��ռ��ʱ�䣬����������������������Դʧ�ܵ���ʱ������Ϊ��С��Ϊ�˱�֤
    // ����Ƶ�ʵ�׼ȷ�ԡ�
    uint32_t min_period;
    struct sc_list task_list; // for TaskNode_t
    struct sc_list next;      // for self
} CooperativeGroup_t;

#define cooperate_scheduler_EXE_TIMES_INF UINT_MAX

typedef struct tagCooperateScheduler_t
{
    struct sc_list group_list; // for CooperativeGroup_t
} CooperateScheduler_t;

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
 * @brief ���������ע�ᵽcooperate_scheduler�С�
 *
 * @param sche_node
 * @retval true ע��ɹ���false ע��ʧ�ܡ�
 */
bool cooperate_scheduler_group_register(CooperativeGroup_t *group);

/**
 * @brief �����Ƿ���ע���״̬��
 *
 * @param sche_node
 * @retval true ��Ȼ��ע��ģ�false δע�ᡣ
 */
bool cooperate_scheduler_is_group_registered(CooperativeGroup_t *group);

/**
 * @brief ȡ���Ѿ�ע�ᵽcooperate_scheduler�е������顣�����������ı�
 * ������������ִ�еļ�¼��
 *
 * @param group
 * @return true �������ע�������������ԭ����ȡ��ע��ʧ�ܡ�
 * @return false ����δע�������������ԭ��ȡ��ע��ɹ���
 */
bool cooperate_scheduler_group_unregister(CooperativeGroup_t *group);

/**
 * @brief Scheduler��ʼ����
 *
 */
void cooperate_scheduler_group_init(CooperativeGroup_t *group);

/**
 * @brief ������������������С��Դռ��ʱ�䡣��������������С��Դռ��ʱ��ָ����
 * �������е����񶼻�ռ��ͬһ��ȫ����Դ��ÿ������ռ�õ�ʱ����ܲ�һ������������
 * һ����С��ռ��ʱ�䣬����������������������Դʧ�ܵ���ʱ������Ϊ��С��Ϊ�˱�֤
 * ����Ƶ�ʵ�׼ȷ�ԡ�
 * 
 * @param group 
 * @param ms >=0
 */
void cooperate_group_set_min_resoure_occupation_time(CooperativeGroup_t* group,uint32_t ms);

/**
 * @brief ������ڵ�ע�ᵽЭͬ���С�
 *
 * @param group Эͬ�����ָ�롣
 * @param task ����ָ�롣
 * @return true ע��ɹ���
 * @return false ע��ʧ�ܡ�
 */
bool cooperate_group_register(CooperativeGroup_t *group, TaskNode_t *task);

/**
 * @brief �ж�����ڵ��Ƿ��Ѿ�ע�ᵽЭͬ���С�
 *
 * @param group Эͬ�����ָ�롣
 * @param task ����ָ�롣
 * @return true ���������С�
 * @return false �����������С�
 */
bool cooperate_group_is_task_registered(CooperativeGroup_t *group, TaskNode_t *task);

/**
 * @brief ȡ���Ѿ�ע�ᵽ�������е����������������ı�
 * ������������ִ�еļ�¼��
 * @param task
 * @return true �������ע�������������ԭ����ȡ��ע��ʧ�ܡ�
 * @return false ����δע�������������ԭ��ȡ��ע��ɹ���
 */
bool cooperate_group_unregister(CooperativeGroup_t *group, TaskNode_t *task);

/**
 * @brief ��������ڵ��Ƶ�ʡ�
 *
 * @param task
 * @param freq 1-1000Hz
 * @return None
 */
void cooperate_scheduler_set_task_freq(TaskNode_t *task, int freq);

#endif //! cooperate_scheduler_H