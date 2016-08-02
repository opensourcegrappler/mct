#include <cairo.h>
#include <math.h>
#include <stdio.h>

#define WIDTH 200
#define HEIGHT 200




int draw_roll_gauge(float degrees,
                    int framecount,
                    float speed,
                    float pspeed,
                    float bearing,
                    float pbearing)
{

    //the fps is only used calculating which frames
    //to apply the sync light to, no other effect on output
    int fps = 5;

    cairo_surface_t *surface;
    cairo_t *cr;
    
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,WIDTH,HEIGHT);
    cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 1, 1, 1);

    int lw = 15;
    cairo_set_line_width(cr, lw);
    cairo_set_line_cap(cr,CAIRO_LINE_CAP_ROUND);
        
        
    cairo_arc(cr, WIDTH/2,HEIGHT/2,100-lw,0.75*M_PI,0.25*M_PI);
    cairo_stroke(cr);
    
    cairo_arc(cr,WIDTH/2,HEIGHT/2,20,0,2*M_PI);
    cairo_fill(cr);
    
    cairo_move_to(cr, WIDTH/2,HEIGHT/2);
    
 
    float theta = -1*(degrees+180)*(M_PI/180);
    float x = (45)*sin(theta);
    float y = (45)*cos(theta);
    
    cairo_rel_line_to(cr,x,y);
    cairo_move_to(cr, WIDTH/2,HEIGHT/2);
    cairo_rel_line_to(cr,-x,-y);
    
    cairo_move_to(cr, (WIDTH/2)+x,(HEIGHT/2)+y);
    
    int clipon_len = 23;
    float hl = clipon_len*sin(theta);
    float hr = -clipon_len*cos(theta);
    
    cairo_set_line_width(cr, 15);
    cairo_rel_line_to(cr,hr,hl);
    
    cairo_move_to(cr, (WIDTH/2)+x,(HEIGHT/2)+y);
    cairo_rel_line_to(cr,-hr,-hl);
    cairo_stroke(cr);
    
    
    cairo_select_font_face(cr, "sans",CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr,35);
    cairo_move_to(cr, WIDTH/2.55,HEIGHT/1.1);
    
    char sdeg[2];

    sprintf(sdeg,"%02.0f",degrees);
    cairo_show_text(cr,sdeg);
    
    if ((framecount<fps) || ((2*fps <= framecount) && (framecount < 3*fps)))
    {
        cairo_set_source_rgb(cr,1,0,0);
        cairo_arc(cr,WIDTH/8,HEIGHT/8,WIDTH/20,0,2*M_PI);
        cairo_fill(cr);

    }
    
    char fh[15];

    printf("Frame: %d, Lean: %02.0f, Speed: %f, PSpeed: %f, Bearing %f, PBearing: %f\n",framecount,
           degrees,
           speed,
           pspeed,
           bearing,
           pbearing);






    sprintf(fh,"frame%06d.png",framecount);
    
    cairo_surface_write_to_png(surface,fh);
    
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    
    return 0;
}
