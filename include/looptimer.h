#ifndef __LOOPTIMER_H
#define __LOOPTIMER_H

#include <cstdio>
#include <iostream>
#include <sys/time.h>

using namespace std;

#define TIMESTAMP struct timeval
#define GETTIME(t) gettimeofday(t, NULL)

#define TOTAL 0

class loop_timer
{
private:
    const char* head_message;
    const char* tail_message;
	const char** headers;

private:
    int count;

private:
    TIMESTAMP *ts1, *ts2;
    int n_timers;

public:
    int step;

private:
    long long* total_time;
    long long prev_total;

private:
    void accumulate(int n)
    {
        long long cur_time = (ts2[n].tv_sec - ts1[n].tv_sec) * 1000000 + (ts2[n].tv_usec - ts1[n].tv_usec);
        total_time[n] += cur_time;
    }

    void print()
    {
        cout << "\r                                                                              \r";
        //fprintf(stdout, "\r");

        int percent = count < 0 ? step : step*100/count;
        //pretty print -_-;
        fprintf(stdout, "%s[", head_message);

        if(percent < 10) fprintf(stdout, "  ");
        else if(percent < 100) fprintf(stdout, " "); 

        if(count < 0)
            fprintf(stdout, "%d]: ", step);
        else
            fprintf(stdout, "%d%%]: ", percent);

        for(int i = 0; i < n_timers; i++){
			if(i == 1) fprintf(stdout, "(");
			if(i > 0) fprintf(stdout, "%s:", headers[i-1]);
            fprintf(stdout, "%lld.%03llds", total_time[i]/1000000, (total_time[i] % 1000000)/1000);
            if(i < n_timers - 1) fprintf(stdout, " ");
			if(i >= 1 && i == n_timers - 1) fprintf(stdout, ")");
        }

        fflush(stdout);
    }

public:
    loop_timer(const char* head, const char* tail, int count, const char** headers, int n = 1){
        this->n_timers = n;
        this->count = count;
        this->head_message = head;
        this->tail_message = tail;
		this->headers = headers;

        ts1 = new TIMESTAMP[n_timers];
        ts2 = new TIMESTAMP[n_timers];
        total_time = new long long[n_timers];

        reset();
    }

    ~loop_timer(){
        delete [] ts1;
        delete [] ts2;
        delete [] total_time;
    }

    void reset(const char* head = NULL, const char* tail = NULL, int count = -1){

        if(head != NULL) this->head_message = head;
        if(tail != NULL) this->tail_message = tail;
        if(count != -1) this->count = count;
        
        step = 0;
        prev_total = 0;
        for(int i = 0; i < n_timers; i++) total_time[i] = 0;
    }

    void start(int n = TOTAL){ 
        if(n == TOTAL){
            step = 0; 
            print();
        }

        GETTIME(&ts1[n]);
    }

    void skip(){ count--; }

    void next(bool verbose = false, int n = TOTAL){
        GETTIME(&ts2[n]);
        accumulate(n);

        if(n == TOTAL){
            step += 1; 
            if((total_time[TOTAL] - prev_total) / 1000 > 400){
                prev_total = total_time[TOTAL];
                print();
            }
			else if(verbose) print();
        }

        GETTIME(&ts1[n]);
    }

    void pseudo_next(int n = TOTAL){ GETTIME(&ts1[n]); }

    void stop(int n = TOTAL)
    {
        GETTIME(&ts2[n]);
        accumulate(n);
    }

    long long end(bool newline = false)
    {
        cout << "\r                                                                                      \r";
        cout.flush();
        fprintf(stderr, "%s[100%%]: ", head_message); fflush(stderr);

        for(int i = 0; i < n_timers; i++){
			if(i == 1) fprintf(stdout, "(");
			if(i > 0) fprintf(stdout, "%s:", headers[i-1]);
            fprintf(stdout, "%lld.%03llds", total_time[i]/1000000, (total_time[i] % 1000000)/1000); fflush(stdout);
            if(i < n_timers - 1){ fprintf(stderr, " "); fflush(stderr); }
			if(i >= 1 && i == n_timers - 1) fprintf(stdout, ")");
        }

        cout << " \t" << tail_message <<" # queries:" << step << flush;

        if(newline) cout << endl << flush;

		return total_time[0];
    }
};

#endif // _LOOPTIMER_H
