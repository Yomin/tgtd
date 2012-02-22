/*
 * This source file is part of the tgtd c application.
 * 
 * Copyright (c) 2012 Martin Rödel aka Yomin
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */

#include <rj.h>
#include <ncurses.h>
#include <string.h>

#define CONF_MAIN_X_PERCENT 75
#define CONF_MAIN_Y_PERCENT 70

#define TASK_THIS 0
#define TASK_NEXT 1
#define TASK_PREV 2

int line;
WINDOW *taskA, *taskB, *sideA, *sideB;
WINDOW *work1A, *work1B, *work2A, *work2B, *work3A, *work3B;
char buf[4];

struct load_state
{
    char *uid, *name, *sub;
};

char* pad(char* str)
{
    buf[0] = '0'; buf[1] = '0'; buf[2] = '0';
    strcpy(buf+3-strlen(str), str);
    return buf;
}

void init(int info, char** field, char** value, void* vstate, struct recordjar* rj)
{
    struct load_state* state = (struct load_state*)vstate;
    
    if(!strcmp(*field, "uid")) state->uid = *value;
    else if(!strcmp(*field, "name")) state->name = *value;
    else if(!strcmp(*field, "sub")) state->sub = *value;
    
    if(info & RJ_INFO_FLD_LAST)
    {
        if(state->uid && state->name)
            wprintw(taskB, "%s: %s\n", pad(state->uid), state->name);
        if(state->uid && state->sub)
        {
            void* tmp = rj->rec;
            char* sub = strtok(state->sub, " ");
            while(sub)
            {
                char* parent = rj_get("uid", sub, "parent", 0, rj);
                if(!parent)
                    rj_add("uid", sub, "parent", state->uid, rj);
                else
                    rj_app("uid", sub, "parent", state->uid, " ", rj);
                sub = strtok(0, " ");
                if(sub)
                    sub[-1] = ' ';
            }
            rj->rec = tmp;
        }
        state->name = 0;
        state->sub = 0;
        state->uid = 0;
    }
}

void screen_init()
{
    initscr();
    keypad(stdscr, 1);
    noecho();
    int mainw = (COLS*CONF_MAIN_X_PERCENT)/100;
    int mainh = (LINES*CONF_MAIN_Y_PERCENT)/100;
    taskA = newwin(mainh, mainw, 0, 0);
    taskB = newwin(mainh-2, mainw-2, 1, 1);
    sideA = newwin(mainh, COLS-mainw, 0, mainw);
    sideB = newwin(mainh-2, COLS-mainw-2, 1, mainw+1);
    work1A = newwin(LINES-mainh, 2*mainw-COLS, mainh, 0);
    work1B = newwin(LINES-mainh-2, 2*mainw-COLS-2, mainh+1, 1);
    work2A = newwin(LINES-mainh, COLS-mainw, mainh, 2*mainw-COLS);
    work2B = newwin(LINES-mainh-2, COLS-mainw-2, mainh+1, 2*mainw-COLS+1);
    work3A = newwin(LINES-mainh, COLS-mainw, mainh, mainw);
    work3B = newwin(LINES-mainh-2, COLS-mainw-2, mainh+1, mainw+1);
    box(taskA, '|', '=');
    box(sideA, '|', '=');
    box(work1A, '|', '=');
    box(work2A, '|', '=');
    box(work3A, '|', '=');
    mvwprintw(taskA, 0, 4, "[ Tasks ]");
    mvwprintw(sideA, 0, 4, "[ Tags ]");
    mvwprintw(work1A, 0, 4, "[ Details ]");
    mvwprintw(work2A, 0, 4, "[ Children ]");
    mvwprintw(work3A, 0, 4, "[ Parents ]");
    refresh();
    wrefresh(taskA);
    wrefresh(sideA);
    wrefresh(work1A);
    wrefresh(work2A);
    wrefresh(work3A);
}

void task_show(int mode, struct recordjar* rj)
{
    char *uid, *name, *desc;
    int show = 0, prevline = line;
    switch(mode)
    {
        case TASK_THIS:
            uid = rj_get(0, 0, "uid", "error", rj);
            show = 1;
            break;
        case TASK_NEXT:
            if(line < rj->size-1)
            {
                ++line;
                uid = rj_get_next(0, 0, "uid", "error", rj);
                show = 1;
            }
            break;
        case TASK_PREV:
            if(line > 0)
            {
                --line;
                uid = rj_get_prev(0, 0, "uid", "error", rj);
                show = 1;
            }
            break;
    }
    if(show)
    {
        mvwchgat(taskB, prevline, 0, -1, A_NORMAL, 0, 0);
        mvwchgat(taskB, line, 0, -1, A_REVERSE, 0, 0);
        wrefresh(taskB);
        name = rj_get("uid", uid, "name", "error", rj);
        desc = rj_get("uid", uid, "desc", "error", rj);
        
        wclear(work1B);
        wprintw(work1B, "%s", desc);
        wrefresh(work1B);
        
        void* tmp = rj->rec;
        
        char* sub = rj_get("uid", uid, "sub", 0, rj);
        if(sub)
            sub = strtok(sub, " ");
        wclear(work2B);
        while(sub)
        {
            name = rj_get("uid", sub, "name", "error", rj);
            wprintw(work2B, "%s: %s\n", pad(sub), name);
            sub = strtok(0, " ");
            if(sub)
                sub[-1] = ' ';
        }
        wrefresh(work2B);
        
        char* parent = rj_get("uid", uid, "parent", 0, rj);
        if(parent)
            parent = strtok(parent, " ");
        wclear(work3B);
        while(parent)
        {
            name = rj_get("uid", parent, "name", "error", rj);
            wprintw(work3B, "%s: %s\n", pad(parent), name);
            parent = strtok(0, " ");
            if(parent)
                parent[-1] = ' ';
        }
        wrefresh(work3B);
        
        rj->rec = tmp;
    }
}

int main(int argc, char* argv[])
{
    screen_init();
    
    struct recordjar rj;
    rj_load("tasks.rj", &rj);
    
    struct load_state state;
    state.name = 0;
    state.sub = 0;
    state.uid = 0;
    if(rj.size > 0)
        rj_mapfold(init, &state, &rj);
    
    line = 0;
    
    curs_set(0);
    task_show(TASK_THIS, &rj);
    
    int c = ' ';
    while(c != 'q')
    {
        c = getch();
        if(c == KEY_UP) task_show(TASK_PREV, &rj);
        else if(c == KEY_DOWN) task_show(TASK_NEXT, &rj);
    }
    
    rj_free(&rj);
    endwin();
    return 0;
}
