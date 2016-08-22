#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "draw.h"

//5Hz data
#define datarate 5

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

// this is hacky, todo remove these globals
inertial P0;
inertial P1;

float calc_yaw(float currentbearing,float prevbearing,float turn)
{
    float yaw = (currentbearing - prevbearing); 
    if (abs(yaw) > (180))
    {
        yaw = turn * ((360) - abs(yaw));
    }

    return yaw;

}


int line_parser(char* lline)
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

        if ((checkdone == 0) && (*c != '$'))
        {
            chk^=*c;
        }
        
        //found the start of a sentence
        if (*c == '$'){
            state = 1;
        }

        //append a valid character to the current word
        if ((state==1) && (*c != ',') && (*c != '\n') && (*c!='*') && (*c!='\r'))
        {
            word[wl] = *c;
            wl++;
        }
        else
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

            switch(wc){
            case 2:
                P0.time = atof(word);
                break;
            case 3:
                P0.status = word[0];
                break;
            case 4:
                P0.lat = atof(word);
                break;
            case 6:
                P0.lon = atof(word);
                break;
            case 8:
                P0.speed = 0.5144*atof(word);
                break;
            case 9:
                P0.bearing = atof(word);
                break;
            case 14:
                strcpy(data_chk,word);
            }
            state = 1;
        }
        c++;
    }

    //convert the calculated checksum to a string for comparison
    sprintf(schk,"%02X",chk);

    if ((!strcmp(data_chk,schk)))
    {
        //for debug by printf only
        //printf("$GPMCT %s %c %s %s %s %s\n",time,status,lat,lon,speed,bearing);
        return 0;
    }
    else
    {
        //printf("Invalid\n");
        //printf("Log: %s, Calculated: %s\n",data_chk,schk);
        return 1;
    }

}

float roll_calc()
{
    //calc cross product to determin if turn is left or right
    float turn = cos(P1.bearing*M_PI/180)*sin(P0.bearing*M_PI/180)
        - sin(P1.bearing*M_PI/180)*cos(P0.bearing*M_PI/180);

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

    float timed = P0.time - P1.time;
//    printf("cur%.2f prev%.2f diff%.2f\n",P0.time,P1.time,timed);

    //use the time difference to set the yaw rate correctly
    

    float yaw = calc_yaw(P0.bearing,P1.bearing,turn);

    float period = 360/(yaw*datarate);
    float ave_speed = (P1.speed+P0.speed)/2;

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
    
    while((read = getline(&line, &len, fh)) != -1)
    {

        if ((line[0] == '$') &&
            (line[1] == 'G') &&
            (line[2] == 'P') &&
            (line[3] == 'R') &&
            (line[4] == 'M') &&
            (line[5] == 'C'))
        {
            //call the line parser
            line_retval = line_parser(line);
            
            if (!line_retval)
            {
                P0.roll = roll_calc();

                R.roll4 = P0.roll;

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
                
                printf("%d %f %f\n",count,R.roll,degrees);

                //after the initial delay draw things
                if (count > lpf_delay)
                {
                    draw_roll_gauge(degrees,count-lpf_delay-1);
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

                //for roll calc, todo remove these globals
                P1 = P0;
            }
        }
    }
    return 0;
}


