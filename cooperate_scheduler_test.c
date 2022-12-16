/**
 * @file cooperate_scheduler_test.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-12-14
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */

#include "cooperate_scheduler_test.h"
#include "cpu_time.h"
#include <stdio.h>

/*
task1��ִ��ʮ��fun1,1000msִ��һ��
task2��ִ��1��fun2��1000msִ��һ�Σ�ȡ��ע��task1
task3��һֱִ��fun3,1msһ�Σ�func3ÿִ��1000�δ�ӡ���һ�Ρ�

����fun2��fun1ִ�е�������ͬ��fun1��fun2�б�ֹͣ����ôfun1ִ�����Ρ�һ�λ���0��ȡ����
�����ڲ�task list�е�λ�á��ܵ���˵��������֮���������ʱ������֮���໥ȡ��ע��Ĳ�����
�����޷�Ԥ������д�����������������Ҫ����һ�ε��������Σ�յġ�

���Խ�����£�
fun3���ж�Ϊif (i == 20)
[cooperate_scheduler Test]: fun3:exe times: 20
[cooperate_scheduler Test]: fun1:exe times: 1
[cooperate_scheduler Test]: fun2:exe times: 1

fun3���ж�Ϊif (i % 1000 == 0)
if (i % 1000 == 0)
[cooperate_scheduler Test]: fun3:exe times: 0
[cooperate_scheduler Test]: fun2:exe times: 1
[cooperate_scheduler Test]: fun3:exe times: 1000
[cooperate_scheduler Test]: fun3:exe times: 2000
[cooperate_scheduler Test]: fun3:exe times: 3000
*/
static TaskNode_t *gTask = NULL;

static bool fun1(void *arg)
{
    TaskNode_t *task_self = (TaskNode_t *)arg;
    static int i = 0;
    i++;
    printf("[cooperate_scheduler Test]: %s:exe times: %d cpu_tick %llu lpe %llu\r\n", __func__, i, scheduler_get_cpu_tick(), scheduler_get_cpu_tick() - task_self->last_exe_tick);
    return true;
}

static bool fun2(void *arg)
{
    TaskNode_t *task_self = (TaskNode_t *)arg;
    static int i = 0;
    i++;
    printf("[cooperate_scheduler Test]: %s:exe times: %d cpu_tick %llu lpe %llu\r\n", __func__, i, scheduler_get_cpu_tick(), scheduler_get_cpu_tick() - task_self->last_exe_tick);
    return true;
}

static bool fun3(void *arg)
{
    static int i = 0;
    if (i % 1000 == 0)
    {
        printf("[cooperate_scheduler Test]: %s:exe times: %d\r\n", __func__, i);
    }
    i++;
    return true;
}

/**
 * @brief cooperate_scheduler���������ԡ�
 *
 */
void cooperate_scheduler_test1()
{
    cooperate_scheduler_init();
    CooperativeGroup_t group1;
    CooperativeGroup_t group2;
    cooperate_scheduler_group_init(&group1);
    cooperate_scheduler_group_init(&group2);

    cooperate_scheduler_group_register(&group1);
    cooperate_scheduler_group_register(&group2);

    TaskNode_t task1 =
        {
            .name = "fun1",
            .exe_times = 10,
            .period = 1000,
            .fun = {fun1, &task1},
        };
    TaskNode_t task2 =
        {
            .name = "fun2",
            .exe_times = 1,
            .period = 1000,
            .fun = {fun2, &task2},
        };

    gTask = &task1;
    TaskNode_t task3 =
        {
            .name = "fun3",
            .exe_times = cooperate_scheduler_EXE_TIMES_INF,
            .period = 1,
            .fun = {fun3, NULL},
            .exe_cnt = cooperate_scheduler_EXE_TIMES_INF - 10,
        };

    cooperate_group_register(&group1, &task1);
    cooperate_group_register(&group1, &task2);

    while (1)
    {
        cooperate_scheduler_handler();
    }
}

bool global_resource = false; // true��ʾռ����
static bool fun5(void *arg)
{
    bool ret = false;

    TaskNode_t *task_self = (TaskNode_t *)arg;
    static int i = 0;
    i++;
    printf("[cooperate_scheduler Test]: %s:exe succeed times: %d exe fail times: %d cpu_tick %llu lpe %llu global_resource %d\r\n",
           __func__, task_self->exe_cnt+1, task_self->exe_fail_cnt,
           scheduler_get_cpu_tick(), scheduler_get_cpu_tick() - task_self->last_exe_tick,
           global_resource);
    if (global_resource == false)
    {
        global_resource = true;
        ret = true;
    }
    return ret;
}

static bool fun6(void *arg)
{
    bool ret = false;

    TaskNode_t *task_self = (TaskNode_t *)arg;
    static int i = 0;
    i++;

    printf("[cooperate_scheduler Test]: %s:exe succeed times: %d exe fail times: %d cpu_tick %llu lpe %llu global_resource %d\r\n",
           __func__, task_self->exe_cnt+1, task_self->exe_fail_cnt,
           scheduler_get_cpu_tick(), scheduler_get_cpu_tick() - task_self->last_exe_tick,
           global_resource);

    if (global_resource == false)
    {
        global_resource = true;
        ret = true;
    }
    return ret;
}

void cooperate_scheduler_test()
{
    cooperate_scheduler_init();
    CooperativeGroup_t group1;
    CooperativeGroup_t group2;
    cooperate_scheduler_group_init(&group1);
    cooperate_scheduler_group_init(&group2);

    cooperate_scheduler_group_register(&group1);
    cooperate_scheduler_group_register(&group2);
    cooperate_group_set_min_resoure_occupation_time(&group1, 500);
    TaskNode_t task1 =
        {
            .name = "fun5",
            .exe_times = 10,
            .period = 1000,
            .fun = {fun5, &task1},
        };
    TaskNode_t task2 =
        {
            .name = "fun6",
            .exe_times = 10,
            .period = 1000,
            .fun = {fun6, &task2},
        };

    cooperate_group_register(&group1, &task1);
    cooperate_group_register(&group1, &task2);

    uint64_t period_recorder = 0;
    while (1)
    {
        cooperate_scheduler_handler();

        if (period_query_user(&period_recorder, 500))
        {
            if (global_resource == true)
            {
                global_resource = false;
            }
        }
    }
}
