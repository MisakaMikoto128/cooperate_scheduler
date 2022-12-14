/**
 * @file cooperate_scheduler.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-14
 * 
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 * 
 */
#include "cooperate_scheduler.h"
#include "cpu_time.h"
/* Config CPU Tick here */
#define scheduler_get_cpu_tick getCurrentMilliSecTimestamp
static struct sc_list _gcooperate_schedulerList = {NULL, NULL};
static struct sc_list *gcooperate_schedulerList = &_gcooperate_schedulerList;
static float oen_tick_time = 1; // ms

/*
���������ԭ��
���exe_cnt < exe_times
    ��ôÿִ�����һ������exe_cnt++��
����
    ����ǰtick - last_exe_tick > periodִ�����񣬼�ʱ�䵽�˾�ִ�б�����
    ���������Ƿ�ɹ����Ὺʼ��һ�μ�ʱ��Ҳ���Ǹ���last_exe_tick

һ����˵last_exe_tick�ڵ�һ��ע�������������
    ��һ�γ�ʼ����ôlast_exe_tickΪ0�����cpu�Ѿ����г���һ��(����tick����󳬹�1s)
    ����֮ǰע�����task���Ҿ����ϴ�ȡ��ע�ᳬ��һ��(����tick����󳬹�1s)
��ô��cooperate_scheduler_handler�д�����Ӧ��taskʱ�����Ͼ�ִ��һ�Ρ�
*/

void Functional_execute(Functional_t *functional)
{
    if (functional->fun != NULL)
    {
        functional->fun(functional->arg);
    }
}

/**
 * @brief cooperate_scheduler��ʼ����
 *
 */
void cooperate_scheduler_init()
{
    sc_list_init(gcooperate_schedulerList);
}

/**
 * @brief cooperate_scheduler��������
 * ֻ�б�֤cooperate_scheduler_handlerִ��Ƶ�ʴ���cooperate_scheduler��ʱ���ļ�ʱ�ֱ��ʵ�Ƶ�ʲ��ܱ�֤ʱ�����׼ȷ�Ķ�ʱ������ȡ�
 */
void cooperate_scheduler_handler()
{
    struct sc_list *it = NULL;
    cooperate_schedulerTask_t *task = NULL;

    sc_list_foreach(gcooperate_schedulerList, it)
    {
        task = sc_list_entry(it, cooperate_schedulerTask_t, next);
        if (scheduler_get_cpu_tick() >= (task->register_tick + task->delay_before_first_exe))
        {
            if (task->exe_cnt < task->exe_times)
            {
                // ����һ����>=������� > ����ô��1 cpu tick�����ʱ��ʱ������2cpu tickִ��һ�Ρ�
                // ���ﲻ����periodΪ0����Ȼ�ͻ�ʧȥ�������á�
                // ������Ҫ��֤һ����ʵʱ��
                if ((scheduler_get_cpu_tick() - task->last_exe_tick) >= task->period)
                {

                    Functional_execute(&task->fun);
                    task->_elapsed_tick_since_last_exe = scheduler_get_cpu_tick() - task->last_exe_tick;
                    task->_exe_tick_error = task->_elapsed_tick_since_last_exe - task->period;
                    if (task->_exe_tick_error > 0)
                    {
                        task->last_exe_tick = scheduler_get_cpu_tick();
                    }
                    else
                    {
                        task->last_exe_tick += task->period;
                    }
                    task->exe_cnt++;
                    task->exe_cnt = task->exe_cnt == cooperate_scheduler_EXE_TIMES_INF ? 0 : task->exe_cnt;
                }
            }
        }
    }
}

/**
 * @brief ������ڵ�ע�ᵽcooperate_scheduler�С�
 * @note ������������periodΪ0����������periodΪ0����ôperiod���Ϊ1��
 * @param sche_node
 * @retval true ע��ɹ���false ע��ʧ�ܡ�
 */
bool cooperate_scheduler_register(cooperate_schedulerTask_t *task)
{
    bool ret = false;
    if (task != NULL)
    {
        task->register_tick = scheduler_get_cpu_tick();
        task->period = task->period == 0 ? 1 : task->period;
        sc_list_init(&task->next);
        sc_list_add_tail(gcooperate_schedulerList, &task->next);
        ret = true;
    }
    return ret;
}

/**
 * @brief �����Ƿ���ע���״̬��
 *
 * @param sche_node
 * @retval true ��Ȼ��ע��ģ�false δע�ᡣ
 */
bool cooperate_scheduler_is_task_registered(cooperate_schedulerTask_t *task)
{
    bool ret = false;
    if (task != NULL)
    {
        struct sc_list *it = NULL;
        cooperate_schedulerTask_t *_task = NULL;
        struct sc_list *item, *tmp;

        sc_list_foreach_safe(gcooperate_schedulerList, tmp, it)
        {
            _task = sc_list_entry(it, cooperate_schedulerTask_t, next);
            if (_task == task)
            {
                ret = true;
                break;
            }
        }
    }
    return ret;
}

/**
 * @brief ȡ���Ѿ�ע�ᵽcooperate_scheduler�е�����
 * @note �������������Ѿ�ִ�еĴ�����
 * @param task
 * @return true �������ע�������������ԭ����ȡ��ע��ʧ�ܡ�
 * @return false ����δע�������������ԭ��ȡ��ע��ɹ���
 */
bool cooperate_scheduler_unregister(cooperate_schedulerTask_t *task)
{
    bool ret = false;
    if (task != NULL)
    {
        struct sc_list *it = NULL;
        cooperate_schedulerTask_t *_task = NULL;
        struct sc_list *item, *tmp;

        sc_list_foreach_safe(gcooperate_schedulerList, tmp, it)
        {
            _task = sc_list_entry(it, cooperate_schedulerTask_t, next);
            if (_task == task)
            {
                // ���task
                task->exe_cnt = 0;

                sc_list_del(gcooperate_schedulerList, &_task->next);
                ret = true;
                break;
            }
        }
    }
    return ret;
}

/**
 * @brief ��������ڵ��Ƶ�ʡ�
 *
 * @param task
 * @param freq 1-1000Hz
 * @return None
 */
void cooperate_scheduler_set_freq(cooperate_schedulerTask_t *task, int freq)
{
    if (task != NULL)
    {
        task->period = 1000.0f / freq / oen_tick_time;
    }
}

Period_t period_last_exe_tick_table[MAX_PERIOD_ID + 1] = {0};

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
bool period_query(uint8_t period_id, uint32_t period)
{
    bool ret = false;

    // ����һ����>=������� > ����ô��1 cpu tick�����ʱ��ʱ������2cpu tickִ��һ�Ρ�
    // ���ﲻ����periodΪ0����Ȼ�ͻ�ʧȥ�������á�
    if ((scheduler_get_cpu_tick() - period_last_exe_tick_table[period_id]) >= period)
    {
        period_last_exe_tick_table[period_id] = scheduler_get_cpu_tick();
        ret = true;
    }
    return ret;
}

/**
 * @brief ͬperiod_query_user��ֻ��ʱ���¼��һ��uint32_t*ָ��ı����С�
 *
 * @param period_recorder ��¼����ʱ��ı�����ָ�롣
 * @param period ���ڡ�
 * @return true ���ڵ���
 * @return false ����δ����
 */
bool period_query_user(uint32_t *period_recorder, uint32_t period)
{
    bool ret = false;
    // ����һ����>=������� > ����ô��1 cpu tick�����ʱ��ʱ������2cpu tickִ��һ�Ρ�
    // ���ﲻ����periodΪ0����Ȼ�ͻ�ʧȥ�������á�
    if ((scheduler_get_cpu_tick() - *period_recorder) >= period)
    {
        *period_recorder = scheduler_get_cpu_tick();
        ret = true;
    }
    return ret;
}