#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "draw.h"

//5Hz data
#define datarate 5


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
    //calc cross product to determin if turn is left or right
    float turn = cos(prevline.bearing*M_PI/180)*sin(linedata.bearing*M_PI/180)
        - sin(prevline.bearing*M_PI/180)*cos(linedata.bearing*M_PI/180);

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


    float timed = linedata.time - prevline.time;
//    printf("cur%.2f prev%.2f diff%.2f\n",linedata.time,prevline.time,timed);

    //use the time difference to set the yaw rate correctly
    

    float yaw = calc_yaw(linedata.bearing,prevline.bearing,turn);

    float period = 360/(yaw*datarate);
    float ave_speed = (prevline.speed+linedata.speed)/2;

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
            

            //what if there are a bunch of invalid points
            // at the start of the log file since the
            //light has started flashing anyway??
            if (!line_retval)
            {
                //second valid line onward start calculating roll etc
                if (lc > 1)
                {
                    roll = roll_calc();
                    draw_roll_gauge(roll,lc-2,linedata.speed,prevline.speed,linedata.bearing,prevline.bearing);
                }

                //increment the line counter for valid lines only
                lc++;
                prevline = linedata;

                
            }
        }
        
    }
    
    return 0;

}


