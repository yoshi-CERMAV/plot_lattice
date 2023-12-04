//
//  YN_input.h
//  
//
//  Created by Yoshiharu Nishiyama on 20/11/2023.
//

#ifndef YN_input_h
#define YN_input_h
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Output.H>
#include <math.h>
class YN_Float_Input: public Fl_Input
{
public:
    YN_Float_Input(int x, int y, int w, int h,const char * l = 0, double v=0, char const in_fmt[]=NULL ):
    Fl_Input(x, y, w, h, l){
        if(!in_fmt){strncpy(fmt, "%5.2f", 256);}
        else strncpy(fmt, in_fmt, 256);
        setv(v);
        tab_nav(1);
        when(FL_WHEN_ENTER_KEY);
    }
    int handle(int e) {
        int ret = Fl_Input::handle(e);
        if ( e == FL_KEYBOARD && Fl::event_key() == FL_Escape ) exit(0);
        if(active())
            switch (e) {
                case FL_KEYDOWN:
                    switch(Fl::event_key()){
                        case FL_Tab:
                        case FL_Enter:
                            ret = 1;
                            val = atof(value());
                            setv(val);
                            do_callback();
                            break;
                    }
            }
  //      if(!value_set) val = atof(value());
        return(ret);
    }

    void setv(double v){
        val = v;
        set();
        set_changed();
        do_callback();
    }
    void set(){
        char str_val[40];
        snprintf(str_val, 40, fmt, val);
        value(str_val);
    }
    double get(){return val;}
protected:
    double val;
    char fmt[256];
};

class YN_Input: public Fl_Input
{
public:
    YN_Input(int x, int y, int w, int h,const char * l = 0, double v=0, char const in_fmt[]=NULL ):
    Fl_Input(x, y, w, h, l){
        if(!in_fmt){strncpy(fmt, "%5.2f", 256);}
        else strncpy(fmt, in_fmt, 256);
        setv(v);
        tab_nav(1);
        when(FL_WHEN_ENTER_KEY);
        value_set =0;
    }
    
    void setv(double v){
        val = v;
        set();
        value_set =1;
        set_changed();
//        do_callback();
    }
    void set(){
        char str_val[40];
        snprintf(str_val, 40, fmt, val);
        value(str_val);
    }
    
    int handle(int e) {
        int ret = Fl_Input::handle(e);
        if ( e == FL_KEYBOARD && Fl::event_key() == FL_Escape ) exit(0);
        if(active())
            switch (e) {
                case FL_KEYDOWN:
                    switch(Fl::event_key()){
                        case FL_Down:
                            val /=2;
                            setv(val);
                            ret =1;
                            do_callback();
                            break;
                        case FL_Up:
                            val *=2;
                            setv(val);
                            ret = 1;
                            do_callback();
                            break;
                        case FL_Enter:
                            ret = 1;
                            val = atof(value());
                            setv(val);
                            do_callback();
                            break;
                    }
            }
        if(!value_set) val = atof(value());
        return(ret);
    }
    double get(){return val;}
protected:
    double val;
    int value_set ;
    char fmt[256];
};

class YN_Angle_Input: public Fl_Input
{
    const double rad2deg = 180/M_PI;
    const double deg2rad = M_PI/180;
public:
    YN_Angle_Input(int x, int y, int w, int h,const char * l = 0, double v=0, char const in_fmt[]=NULL ):
    Fl_Input(x, y, w, h, l){
        if(!in_fmt){strncpy(fmt, "%5.2f", 256);}
        else strncpy(fmt, in_fmt, 256);
        setv(v);
        tab_nav(1);
        when(FL_WHEN_ENTER_KEY);
        value_set =0;
    }
    void setv(double v){
        val = v;
        set();
        value_set =1;
        set_changed();
        do_callback();
    }
    void set(){
        char str_val[40];
        snprintf(str_val, 40, fmt, val*rad2deg);
        value(str_val);
    }
    
    int handle(int e) {
        int ret = Fl_Input::handle(e);
        if ( e == FL_KEYBOARD && Fl::event_key() == FL_Escape ) exit(0);
        if(active())
            switch (e) {
                case FL_KEYDOWN:
                    switch(Fl::event_key()){
                        case FL_Down:
                            val /=2;
                            setv(val);
                            ret =1;
 //                           do_callback();
                            break;
                        case FL_Up:
                            val *=2;
                            setv(val);
                            ret = 1;
//                            do_callback();
                            break;
                        case FL_Enter:
                            ret = 1;
                            val = atof(value())*deg2rad;
                            setv(val);
 //                           do_callback();
                            break;
                    }
            }
        if(!value_set) val = atof(value())*deg2rad;
        return(ret);
    }
    double get(){return val;}
protected:
    double val;
    int value_set ;
    char fmt[256];
    
};


class YN_Int_Input: public Fl_Int_Input
{
public:
    YN_Int_Input(int x, int y, int w, int h,const char * l = 0, int v=0):
    Fl_Int_Input(x, y, w, h, l){
        set(v);
        tab_nav(1);
    }
    void set(int i){
        ival = i;
        char val[10];
        snprintf(val, 10, "%d", ival);
        value(val);
    }
    void increment(int i){
        ival += i;
        set(ival);
        do_callback();
    }
    void decrement(int i){
        ival -= i;
        set(ival);
        do_callback();
    }
    int val(){return ival;}
    int handle(int e) {
        int ret = Fl_Input::handle(e);
        if(active()){
            switch (e) {
                case FL_KEYDOWN:
                    switch(Fl::event_key()){
                        case FL_Up:
                            increment(1);
                            break;
                        case FL_Down:
                            decrement(1);
                            break;
                        case FL_Enter:
                            ret = 1;
                            ival = atoi(value());
                            do_callback();
                            break;

                    }
            }
        }
        return ret;
    }
protected:
    int ival;
};


#endif /* YN_input_h */
