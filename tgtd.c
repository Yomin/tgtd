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
#define CONF_WORK1_X_PERCENT 50

#define TASK_THIS 0
#define TASK_NEXT 1
#define TASK_PREV 2

int line;
WINDOW *task, *work1, *work2, *work3, *side;

struct load_state
{
    char *uid, *name, *desc, *sub;
};

void init(int info, char** field, char** value, void* vstate, struct recordjar* rj)
{
    struct load_state* state = (struct load_state*)vstate;
    
    if(!strcmp(*field, "uid")) state->uid = *value;
    else if(!strcmp(*field, "name")) state->name = *value;
    else if(!strcmp(*field, "desc")) state->desc = *value;
    else if(!strcmp(*field, "sub")) state->sub = *value;
    
    if(info & RJ_INFO_FLD_LAST)
    {
        if(state->name && state->desc)
            wprintw(task, "%s: %s\n", state->name, state->desc);
        if(state->sub && state->uid)
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
        state->desc = 0;
        state->sub = 0;
        state->uid = 0;
    }
}

void screen_init()
{
    initscr();
    keypad(stdscr, 1);
    int mainw = (COLS*CONF_MAIN_X_PERCENT)/100;
    int mainh = (LINES*CONF_MAIN_Y_PERCENT)/100;
    int work1w = (COLS*CONF_WORK1_X_PERCENT)/100;
    int workw = (COLS-work1w)/2;
    task = newwin(mainh, mainw, 0, 0);
    side = newwin(mainh, COLS-mainw, 0, mainw);
    work1 = newwin(LINES-mainh, work1w, mainh, 0);
    work2 = newwin(LINES-mainh, workw, mainh, work1w+workw*0);
    work3 = newwin(LINES-mainh, workw, mainh, work1w+workw*1);
    refresh();
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
        mvwchgat(task, prevline, 0, -1, A_NORMAL, 0, 0);
        mvwchgat(task, line, 0, -1, A_REVERSE, 0, 0);
        wrefresh(task);
        name = rj_get("uid", uid, "name", "error", rj);
        desc = rj_get("uid", uid, "desc", "error", rj);
        wclear(work1);
        wprintw(work1, "= Task =\n%s: %s\n%s", uid, name, desc);
        wrefresh(work1);
        
        void* tmp = rj->rec;
        
        char* sub = rj_get("uid", uid, "sub", 0, rj);
        if(sub)
            sub = strtok(sub, " ");
        wclear(work2);
        wprintw(work2, "= Children =\n");
        while(sub)
        {
            name = rj_get("uid", sub, "name", "error", rj);
            wprintw(work2, "%s: %s\n", sub, name);
            sub = strtok(0, " ");
            if(sub)
                sub[-1] = ' ';
        }
        wrefresh(work2);
        
        char* parent = rj_get("uid", uid, "parent", 0, rj);
        if(parent)
            parent = strtok(parent, " ");
        wclear(work3);
        wprintw(work3, "= Parents =\n");
        while(parent)
        {
            name = rj_get("uid", parent, "name", "error", rj);
            wprintw(work3, "%s: %s\n", parent, name);
            parent = strtok(0, " ");
            if(parent)
                parent[-1] = ' ';
        }
        wrefresh(work3);
        
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
    state.desc = 0;
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
