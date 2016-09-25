#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "draw.h"


// define the struct type for holding inertial related data
typedef struct datastr {
    float time;
    char status;
    float lat;
    float lon;
    float speed;
    float bearing;
    double roll;
} inertial;


float calc_yaw(float currentbearing,float prevbearing,float turn)
{
    float yaw = (currentbearing - prevbearing); 
    if (abs(yaw) > (180))
    {
        yaw = turn * ((360) - abs(yaw));
    }

    return yaw;
}


int line_parser(char* lline,inertial *current)
{

    char* c = lline;
    int state = 0;
    char word[10];
    int wl = 0;
    int wc = 0;
    int checkdone = 0;
    char data_chk[3];
    char schk[3];
    int chk = 0;
   
    while (*c)
    {

        if (*c == '*')
        {
            checkdone = 1;
        }

        if ((checkdone == 0) && (state==1))
        {
            chk^=*c;
        }
        
        //found the start of a sentence
        if (*c == '$'){
            state = 1;
        }

        //append a valid character to the current word
        if ((state==1)   &&
            (*c != ',' ) &&
            (*c != '\n') &&
            (*c != '*' ) &&
            (*c != '\r'))
        {
            word[wl] = *c;
            wl++;
        }
        else if ((state==1) &&
                 (*c == ',' ) ||
                 (*c == '\n') ||
                 (*c == '*' ) ||
                 (*c == '\r'))
        {
            state = 2;
        }

        //word is complete, do something with it
        if (state==2)
        {
            //terminate the word character array
            word[wl++] = '\0';
            //reset the wordlength and increment the wordcounter
            wl = 0;
            wc++;

            //assign fields
            switch(wc){
            case 1:
                if (strcmp(word,"$GPRMC"))
                {
                    state = 0;
                    return 1;
                }
                break;
            case 2:
                current->time = atof(word);
                break;
            case 3:
                current->status = word[0];
                break;
            case 4:
                current->lat = atof(word);
                break;
            case 6:
                current->lon = atof(word);
                break;
            case 8:
                current->speed = 0.5144*atof(word);
                break;
            case 9:
                current->bearing = atof(word);
                break;
            case 14:
                strcpy(data_chk,word);
                state = 0;
            }
            state = 1;
        }
        c++;
    }

    //convert the calculated checksum to a string for comparison
    sprintf(schk,"%02X",chk);
    if ((!strcmp(data_chk,schk))) return 0;
    else return 1;

}

float roll_calc(inertial *current,inertial *prev, int datarate)
{

    float cur_bearing,prev_bearing,cur_speed,prev_speed,prev_roll;
    int cur_time,prev_time;

    cur_bearing = current->bearing;
    prev_bearing = prev->bearing;
    cur_speed = current->speed;
    prev_speed = prev->speed;
    prev_time = 10*(prev->time);
    cur_time = 10*(current->time);

    int td = cur_time-prev_time;

    //check time diff between datapoints is as expected 
    if (td!=(10/datarate))
    {
        //return previous value of roll if not
        return prev->roll; 
    }
    
    //calc cross product to determin if turn is left or right
    float turn = cos(prev_bearing*M_PI/180)*sin(cur_bearing*M_PI/180)
        - sin(prev_bearing*M_PI/180)*cos(cur_bearing*M_PI/180);

    //right
    if (turn >= 0)
    {
        turn = 1;
    }
    //left
    else
    {
        turn = -1;
    }

    float yaw = calc_yaw(cur_bearing,prev_bearing,turn);

    float period = 360/(yaw*datarate);
    float ave_speed = (prev_speed+cur_speed)/2;

    if (ave_speed <1)
    {
        return 0.0;
    }
    
    float radius = ave_speed * period/(2*M_PI);
    float roll = atan(ave_speed*ave_speed/(radius*9.8))*180/M_PI;

    return roll;
}


int main(int argc, char *argv[])
{

    FILE *fh;

    fh = fopen(argv[1],"r");

    int datarate;
    
    if (argv[2]) datarate = atoi(argv[2]);
    else datarate = 5;
   
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int line_retval;

    int count =0;
    double degrees = 0;

    typedef struct datastr {
        double proll4;
        double proll3;
        double proll2;
        double proll1;
        double roll;
        double roll1;
        double roll2;
        double roll3;
        double roll4;
    } rollstr;

    rollstr R;

    //lpf phase delay, 4 frames = N/2 from FIR filter
    int lpf_delay = 4;
    
    float a0 = 0.227272727;
    float a1 = 0.1966520727;
    float a2 = 0.12272727272;
    float a3 = 0.0488024;
    float a4 = 0.018181818;

    //the current and previous datapoints
    inertial current_t;
    inertial previous_t;

    
    while((read = getline(&line, &len, fh)) != -1)
    {
        //call the line parser
        line_retval = line_parser(line,&current_t);
        
        if (!line_retval)
        {
            current_t.roll = roll_calc(&current_t,&previous_t,datarate);
            
            R.roll4 = current_t.roll;

            //do filtering here
            degrees = (a0*R.roll) +
                (a1*R.roll1)  +
                (a1*R.proll1) +
                (a2*R.roll2)  +
                (a2*R.proll2) +
                (a3*R.roll3)  +
                (a3*R.proll3) +
                (a4*R.roll4)  +
                (a4*R.proll4);
                
            printf("%d %f %f %f\n",count,current_t.roll,previous_t.roll,degrees);

            //after the initial delay draw things
            if (count > lpf_delay)
            {
                draw_roll_gauge(degrees,count-lpf_delay-1,datarate);
            }
            
            //increment the frame counter
            count++;
            
            //shuffle roll values along the buffer
            R.proll4 = R.proll3;
            R.proll3 = R.proll2;
            R.proll2 = R.proll1;
            R.proll1 = R.roll;
            R.roll = R.roll1;
            R.roll1 = R.roll2;
            R.roll2 = R.roll3;
            R.roll3 = R.roll4;
            
            //the current datapoint becomes the previous datapoint
            previous_t = current_t;
        }
    }
    return 0;
}


