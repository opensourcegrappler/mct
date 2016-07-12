#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "draw.h"

struct datastr {
    float time;
    char status;
    float lat;
    float lon;
    float speed;
    float bearing;
};

struct datastr linedata;
struct datastr prevline;


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
                linedata.time = atof(word);
                break;
            case 3:
                linedata.status = word[0];
                break;
            case 4:
                linedata.lat = atof(word);
                break;
            case 6:
                linedata.lon = atof(word);
                break;
            case 8:
                linedata.speed = 0.5144*atof(word);
                break;
            case 9:
                linedata.bearing = atof(word);
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
    
    if ((linedata.status == 'A') && (!strcmp(data_chk,schk)))
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

    float turn = cos(prevline.bearing*M_PI/180)*sin(linedata.bearing*M_PI/180)
        - sin(prevline.bearing*M_PI/180)*cos(linedata.bearing*M_PI/180);

//    printf("turnb %f\n",turn);
    
    if (turn >= 0)
    {
        turn = 1;
    }
    else
    {
        turn = -1;
    }
//    printf("turna %f\n",turn);
   
    float yaw_rate = linedata.bearing - prevline.bearing;
    if (abs(yaw_rate) > 180)
    {
        yaw_rate = turn * (360 - abs(yaw_rate));
    }
//    printf("yaw rate %f\n",yaw_rate);

    float period = 360/yaw_rate; //assumes 1Hz data
//    printf("period %f\n",period);
    float ave_speed = (prevline.speed+linedata.speed)/2;
    if (ave_speed == 0)
    {
        return 0.0;
    }
    
//    printf("ave speed %f\n",ave_speed);
    float radius = ave_speed * period/(2*M_PI);
//    printf("radius %f\n",radius);
    float roll = atan(ave_speed*ave_speed/(radius*9.8))*180/M_PI;
//    printf("roll %f\n",roll);

    
//    printf("%f %f %f\n",prevline.bearing,linedata.bearing,turn);
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
    int lc=1;

    float roll;
    
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
                //second valid line onward start calculating roll etc
                if (lc > 1)
                {
                    roll = roll_calc();
                    draw_roll_gauge(roll,lc);
//                    printf("%f %d main\n",roll,lc);
                }

                //increment the line counter for valid lines only
                lc++;
                prevline = linedata;

                
            }
        }
        
    }
    
    //parse the gps log file write this in another file, no other file
    //calc the roll at each data point and produce a png frame

    //catmull rom interpolate the data?
    

    
/*
    turn = cos((fbearing[0])*pi/180)*sin(fbearing[1])*.pi/180)
    - sin(fbearing[0]*pi/180)*cos((fbearing[1])*pi/180)

    if (turn > 0)
    {
        turn = -1;
    } //left
    else
    {
        turn = 1;
    }
 
    yaw_rate = bearing[1] - bearing[2];
    if (yaw_rate > 180)
    {
        yaw_rate = 360 - yaw_rate;
    }
    period = turn * 360/yaw_rate;
    ave_speed = (speed[0]+speed[1])/2;
    radius = average_speed * period/2*pi;
    roll = arctan(average_speed*average_speed/(radius*9.8))*180/pi;

*/

    int i;
    int count = 0;
    for(i=-90;i<91;i++)
    {
//        draw_roll_gauge(i,count);
        count++;
    }
    return 0;

}


