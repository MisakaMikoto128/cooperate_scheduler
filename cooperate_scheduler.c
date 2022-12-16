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

static CooperateScheduler_t _gcooperate_scheduler;

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

/**
 * @brief һ��Groupͨ����һϵ��ռ��ͬһ��Դ�ķ���������������ص�true��ʾ�������ִ�е���ʱ������Դ��Ȼæµ��
 * ����ʱ���Ǽ������⣺
 *  1. �ϴ�ִ��ʱ�̵����ε��ȵ�ʱ��ʱ������ִ������
 *  2. һ�����ڵ�task���ȵ㼸������ͬһʱ�̣�����ִ��ʧ�ܵĴ��������ӣ���������ִ��ʧ�ܵĴ���
 *  3. ����ִ��Ƶ���Ƿ����Ԥ�ڣ�ִ�м���Ƿ��㹻����
 * @param functional
 * @return true
 * @return false
 */
bool BoolFunctional_execute(BoolFunctional_t *functional)
{
    bool exe_source_free = false;
    if (functional->fun != NULL)
    {
        exe_source_free = functional->fun(functional->arg);
    }
    return exe_source_free;
}

/**
 * @brief cooperate_scheduler��ʼ����
 *
 */
void cooperate_scheduler_init()
{
    sc_list_init(&_gcooperate_scheduler.group_list);
}

/**
 * @brief cooperate_scheduler��������
 * ֻ�б�֤cooperate_scheduler_handlerִ��Ƶ�ʴ���cooperate_scheduler��ʱ���ļ�ʱ�ֱ��ʵ�Ƶ�ʲ��ܱ�֤ʱ�����׼ȷ�Ķ�ʱ������ȡ�
 */
void cooperate_scheduler_handler()
{
    static struct sc_list *it = NULL;
    static CooperativeGroup_t *group = NULL;
    static const struct sc_list *group_list = &_gcooperate_scheduler.group_list;

    static struct sc_list *it_task = NULL;
    static TaskNode_t *task = NULL;
    // Group�����õ���Դ�Ƿ��ǿ��е�
    static bool exe_source_free = false;
    static struct sc_list *poped = NULL;
    sc_list_foreach(group_list, it)
    {
        group = sc_list_entry(it, CooperativeGroup_t, next);
        sc_list_foreach(&group->task_list, it_task)
        {
            task = sc_list_entry(it_task, TaskNode_t, next);
            if (scheduler_get_cpu_tick() >= (task->register_tick + task->delay_before_first_exe))
            {
                if (task->exe_cnt < task->exe_times)
                {
                    // ����һ����>=������� > ����ô��1 cpu tick�����ʱ��ʱ������2cpu tickִ��һ�Ρ�
                    // ���ﲻ����periodΪ0����Ȼ�ͻ�ʧȥ�������á�
                    // ������Ҫ��֤һ����ʵʱ��
                    if ((scheduler_get_cpu_tick() - task->last_exe_tick) >= task->period)
                    {
                        exe_source_free = BoolFunctional_execute(&task->fun);
                        if (exe_source_free)
                        {
                            task->_elapsed_tick_since_last_exe = scheduler_get_cpu_tick() - task->last_exe_tick;
                            task->_exe_tick_error = task->_elapsed_tick_since_last_exe - 2 * task->period;
                            if (task->_exe_tick_error > 0)
                            {
                                // ���������һ�ε�ִ��ʱ��
                                task->last_exe_tick = scheduler_get_cpu_tick();
                            }
                            else
                            {
                                task->last_exe_tick += task->period;
                            }
                            task->exe_cnt++;
                            task->exe_cnt = task->exe_cnt == cooperate_scheduler_EXE_TIMES_INF ? 0 : task->exe_cnt;

                            poped = sc_list_pop_head(&group->task_list);
                            sc_list_add_tail(&group->task_list, poped);
                        }
                        else
                        {
                            task->_elapsed_tick_since_last_exe = scheduler_get_cpu_tick() - task->last_exe_tick;
                            task->_exe_tick_error = task->_elapsed_tick_since_last_exe - 2 * task->period;
                            if (task->_exe_tick_error > 0)
                            {
                                // ���������һ�ε�ִ��ʱ��
                                task->last_exe_tick = scheduler_get_cpu_tick();
                            }
                            else
                            {
                                // ���������η�����Դæ�Ժ��ȴ�1��tick��ȥִ��Functional_execute����Դ�Ƿ����
                                task->last_exe_tick += group->min_period;
                            }
                            task->exe_fail_cnt++;
                        }
                    }
                }
            }
        }
    }
}

bool cooperate_scheduler_group_register(CooperativeGroup_t *group)
{
    bool ret = false;
    if (group != NULL)
    {
        group->min_period = 1;
        sc_list_init(&group->next);
        sc_list_add_tail(&_gcooperate_scheduler.group_list, &group->next);
        ret = true;
    }
    return ret;
}

bool cooperate_scheduler_is_group_registered(CooperativeGroup_t *group)
{
    bool ret = false;
    if (group != NULL)
    {
        struct sc_list *it = NULL;
        CooperativeGroup_t *_group = NULL;
        struct sc_list *item, *tmp;

        sc_list_foreach_safe(&_gcooperate_scheduler.group_list, tmp, it)
        {
            _group = sc_list_entry(it, CooperativeGroup_t, next);
            if (_group == group)
            {
                ret = true;
                break;
            }
        }
    }
    return ret;
}

bool cooperate_scheduler_unregister(CooperativeGroup_t *group)
{
    bool ret = false;
    if (group != NULL)
    {
        struct sc_list *it = NULL;
        CooperativeGroup_t *_group = NULL;
        struct sc_list *item, *tmp;

        sc_list_foreach_safe(&_gcooperate_scheduler.group_list, tmp, it)
        {
            _group = sc_list_entry(it, CooperativeGroup_t, next);
            if (_group == group)
            {
                sc_list_del(&_gcooperate_scheduler.group_list, &_group->next);
                ret = true;
                break;
            }
        }
    }
    return ret;
}

void cooperate_scheduler_group_init(CooperativeGroup_t *group)
{
    sc_list_init(&group->task_list);
}

/**
 * @brief ������������������С��Դռ��ʱ�䡣��������������С��Դռ��ʱ��ָ����
 * �������е����񶼻�ռ��ͬһ��ȫ����Դ��ÿ������ռ�õ�ʱ����ܲ�һ������������
 * һ����С��ռ��ʱ�䣬����������������������Դʧ�ܵ���ʱ������Ϊ��С��Ϊ�˱�֤
 * ����Ƶ�ʵ�׼ȷ�ԡ�
 * 
 * @param group 
 * @param ms >= 0
 */
void cooperate_group_set_min_resoure_occupation_time(CooperativeGroup_t* group,uint32_t ms)
{
    group->min_period = ms / oen_tick_time;
}

/**
 * @brief ������ڵ�ע�ᵽЭͬ���С�
 *
 * @param group Эͬ�����ָ�롣
 * @param task ����ָ�롣
 * @return true ע��ɹ���
 * @return false ע��ʧ�ܡ�
 */
bool cooperate_group_register(CooperativeGroup_t *group, TaskNode_t *task)
{
    bool ret = false;
    if (task != NULL)
    {
        task->register_tick = scheduler_get_cpu_tick();
        task->period = task->period == 0 ? 1 : task->period;
        sc_list_init(&task->next);
        sc_list_add_tail(&group->task_list, &task->next);
        ret = true;
    }
    return ret;
}

/**
 * @brief �ж�����ڵ��Ƿ��Ѿ�ע�ᵽЭͬ���С�
 *
 * @param group Эͬ�����ָ�롣
 * @param task ����ָ�롣
 * @return true ���������С�
 * @return false �����������С�
 */
bool cooperate_group_is_task_registered(CooperativeGroup_t *group, TaskNode_t *task)
{
    bool ret = false;
    if (task != NULL)
    {
        struct sc_list *it = NULL;
        TaskNode_t *_task = NULL;
        struct sc_list *item, *tmp;

        sc_list_foreach_safe(&group->task_list, tmp, it)
        {
            _task = sc_list_entry(it, TaskNode_t, next);
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
 * @brief ȡ���Ѿ�ע�ᵽ�������е�����
 *
 * @param task
 * @return true �������ע�������������ԭ����ȡ��ע��ʧ�ܡ�
 * @return false ����δע�������������ԭ��ȡ��ע��ɹ���
 */
bool cooperate_group_unregister(CooperativeGroup_t *group, TaskNode_t *task)
{
    bool ret = false;
    if (task != NULL)
    {
        struct sc_list *it = NULL;
        TaskNode_t *_task = NULL;
        struct sc_list *item, *tmp;

        sc_list_foreach_safe(&group->task_list, tmp, it)
        {
            _task = sc_list_entry(it, TaskNode_t, next);
            if (_task == task)
            {
                sc_list_del(&group->task_list, &_task->next);
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
 * @param group
 * @param freq 1-1000Hz
 * @return None
 */
void cooperate_scheduler_set_task_freq(TaskNode_t *task, int freq)
{
    if (task != NULL)
    {
        task->period = 1000.0f / freq / oen_tick_time;
    }
}