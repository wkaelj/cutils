#include "function_timer.h"

#include <stdio.h>
#include <string.h>

#include "types.h"
#include "platform.h"
#include "string_util.h"

#define LINE_LENGTH_BUFFER 512
#define FUNCTION_LENGTH_BUFFER 128
#define FUNCTION_TIMER_CACHE "function_timer.csv"

#define max_value(a, b) (a > b ? a : b)
#define min_value(a, b) (a < b ? a : b)

//
// Types
//

#ifndef FUNCTION_TIMER_NO_DIAGNOSTIC

struct TimerData
{
    char functionName[FUNCTION_LENGTH_BUFFER];
    f64 avgTime;
    f64 maxTime;
    f64 minTime;
    // remember blank
    u64 totalExecutionCount; // used for average
    f64 totalExecutionTime;  // used for average
};

struct FunctionListNode_t
{
    struct TimerData timerData;
    LIST_ENTRY(FunctionListNode_t) data;
};

struct
{
    bool initialized;

    LIST_HEAD(FunctionListHead_t, FunctionListNode_t) functions;

} g_timerData = {};

//
// Helper Declerations
//

// update avg, min and max for an existing element
void add_data_point(struct FunctionListNode_t *node, f64 executionTime);

// add a new element to the list
void add_new_element(const char *name, f64 executionTime);

// check to see if a node has been created, if so get a reference
struct FunctionListNode_t *find_function_data(const char *functionName);

// export the linked list to a file
void export_list_to_file(
    struct FunctionListHead_t *listHead, const char *restrict file);

// convert a TimerData into a string for a .csv file
void create_timer_string(char *restrict buf, struct TimerData data);

// convert the clock_t time data to ms
f64 sec_to_ms(f64 time);

//
// Public methods
//

void __attribute__((constructor)) init_timer(void)
{

    LIST_INIT(&g_timerData.functions);

    g_timerData.initialized = true;
}

void __attribute__((destructor)) terminate_timer(void)
{
    // TODO

    // write data to file
    export_list_to_file(&g_timerData.functions, FUNCTION_TIMER_CACHE);

    // delete list and free memory
    struct FunctionListNode_t *np;
    while (!LIST_EMPTY(&g_timerData.functions))
    {
        np = LIST_FIRST(&g_timerData.functions);
        LIST_REMOVE(np, data);
        free(np);
    };

    g_timerData.initialized = false;
}

struct FunctionTimerData start_timer(const char *func)
{

    if (!g_timerData.initialized)
    {
        printf("function_timer: Timer has not been initialized\n");
        return (struct FunctionTimerData){0};
    }

    struct FunctionTimerData t = {};

    u32 funcNameLength = FUNCTION_LENGTH_BUFFER;

    // truncate and set name
    cutil_string_truncate(t.functionName, &funcNameLength, func, '(', false);

    // get time
    t.startTime = cutil_platform_get_time();

    return t;
}

void end_timer(struct FunctionTimerData t)
{

    if (!g_timerData.initialized)
    {
        printf("function_timer: Timer has not been initialized\n");
        return;
    }

    // get time clock ticks
    f64 endTime = cutil_platform_get_time();
    if (endTime < t.startTime)
    {
        printf(
            "function_timer: Clock rollover detected, discaring measurement\n");
        return;
    }

    const f64 executionTime = sec_to_ms(endTime - t.startTime);

    struct FunctionListNode_t *node = find_function_data(t.functionName);

    if (node)
    {
        add_data_point(node, executionTime);
    }
    else
    {
        add_new_element(t.functionName, executionTime);
    }
}

//
// Helper implementations
//

void create_timer_string(char *restrict buf, struct TimerData data)
{
    // func,avgTime,maxTime,minTime, ,totalExecutionCount,totalExecutionTime
    snprintf(
        buf,
        LINE_LENGTH_BUFFER,
        "%s,%f,%f,%f\n",
        data.functionName,
        data.avgTime,
        data.maxTime,
        data.minTime);
}

void add_data_point(struct FunctionListNode_t *node, f64 executionTime)
{
    struct TimerData *data = &node->timerData;

    data->totalExecutionTime += executionTime;
    data->totalExecutionCount++;
    data->avgTime = data->totalExecutionTime / data->totalExecutionCount;
    data->maxTime = max_value(data->maxTime, executionTime);
    data->minTime = data->minTime != 0 ? min_value(data->minTime, executionTime)
                                       : executionTime;
}

// add a new element to the list
void add_new_element(const char *name, f64 executionTime)
{
    struct FunctionListNode_t *node = malloc(sizeof(struct FunctionListNode_t));
    node->timerData                 = (struct TimerData){
                        .functionName        = {},
                        .avgTime             = executionTime,
                        .maxTime             = executionTime,
                        .minTime             = executionTime,
                        .totalExecutionCount = 1,
                        .totalExecutionTime  = executionTime};
    strncpy(node->timerData.functionName, name, FUNCTION_LENGTH_BUFFER);

    LIST_INSERT_HEAD(&g_timerData.functions, node, data);
}

struct FunctionListNode_t *find_function_data(const char *functionName)
{
    // search list for element
    struct FunctionListNode_t *np = NULL;
    LIST_FOREACH(np, &g_timerData.functions, data)
    {
        if (strncmp(
                np->timerData.functionName,
                functionName,
                FUNCTION_LENGTH_BUFFER) == 0)
        {
            return np;
        }
    }

    return NULL;
}

// export the linked list to a file
void export_list_to_file(
    struct FunctionListHead_t *listHead, const char *restrict filepath)
{

    u32 pathLength = 0;
    cutil_platform_localize_file_name(NULL, filepath, &pathLength);
    char path[pathLength];
    cutil_platform_localize_file_name(path, filepath, &pathLength);

    FILE *file = fopen(path, "w");
    fprintf(file, "Function Name,Avg Time(CPU ticks),Max Time, Min Time\n");
    fprintf(file, "Function Name,Avg Time(ms),Max Time(ms), Min Time(ms)\n");

    printf("Data:");
    // write each element to string
    char lineBuf[LINE_LENGTH_BUFFER];
    struct FunctionListNode_t *np = NULL;
    LIST_FOREACH(np, listHead, data)
    {
        create_timer_string(lineBuf, np->timerData);
        fputs(lineBuf, file);
        printf("\t%s", lineBuf);
    }

    fclose(file);
}

f64 sec_to_ms(f64 time) { return time * 1000; }

#endif // FUNCTION_TIMER_NO_DIAGNOSTIC
