#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Round_Button.H>
#include <YN_input.h>
#include <FL/Fl_Slider.H>
#include <iostream>
using namespace std;
class YN_Min_Max
: public Fl_Group
{
YN_Float_Input *min_input;
Fl_Slider *min_slider;
YN_Float_Input *max_input;
Fl_Slider *max_slider;
    static void min_sl(Fl_Widget *w, void *data) {
        float the_min =((Fl_Slider *)w)->value();
        YN_Min_Max * minmax = (YN_Min_Max *)data;
        float the_max =minmax->max;
        if(the_min > the_max) the_min = the_max;
        ((Fl_Slider *)w)->value(the_min);
        minmax->min = the_min;
        minmax->min_input->setv(the_min);
        minmax->do_callback();
    }
    static void max_sl(Fl_Widget *w, void *data) {
        float the_max =((Fl_Slider *)w)->value();
        YN_Min_Max * minmax = (YN_Min_Max *)data;
        float the_min = minmax->min;
        if(the_min > the_max) the_max = the_min;
        ((Fl_Slider *)w)->value(the_max);
        minmax->max = the_max;
        minmax->max_input->setv(the_max);
        minmax->do_callback();
    }
    static void max_in(Fl_Widget *w, void *data) {
        YN_Float_Input *input =(YN_Float_Input *)w;
        YN_Min_Max * minmax = (YN_Min_Max *)data;
       float the_max = input->get();
        float the_min = minmax->min;
        float max0 = minmax->max0;
  //      cout << the_max << " "<< the_min<<endl;
        if(the_max > max0){
            the_max = max0;
            input->setv(the_max);
        }else
        if(the_min > the_max){
            the_max = the_min;
            input->setv(the_max);
        }else{
            minmax->max_slider->value(the_max);
        }
        minmax->max = the_max;
        minmax->do_callback();
    }
    
    static void min_in(Fl_Widget *w, void *data) {
        YN_Float_Input *input =(YN_Float_Input *)w;
        YN_Min_Max * minmax = (YN_Min_Max *)data;
        float the_min = input->get();
        float the_max = minmax->max;
        float min0 = minmax->min0;
        if(the_min < min0){
            the_min = min0;
            input->setv(the_min);
        }else
        if(the_min > the_max){
            the_min = the_max;
            input->setv(the_min);
        }else{
            minmax->min_slider->value(the_min);
        }
        minmax->min = the_min;
        minmax->do_callback();
    }

public:
    YN_Min_Max(int x, int y, int w, int h, const char *label, float in_min, float in_max)
    : Fl_Group(x, y, w, h, label)
    {
        min = in_min;
        max = in_max;
        min0 = min;
        max0 = max;
        int w0 = 50;
        int w1 = 120;
        int h0 = 25;
        begin();
        min_input = new YN_Float_Input(x, y, w0, h0, "", min, "%5.1f");
        min_input->callback(min_in, this);

        x+= w0;
        min_slider = new Fl_Slider(x, y, 120, 25, "");
        min_slider->type(FL_HORIZONTAL);
        min_slider->selection_color((Fl_Color)1);
        min_slider->labelsize(8);
        min_slider->bounds(min, max);
        min_slider->callback(min_sl, this);
        y+=25;
        max_slider = new Fl_Slider(x, y, 120, 25, "");
        max_slider->type(FL_HORIZONTAL);
        max_slider->selection_color((Fl_Color)1);
        max_slider->labelsize(8);
        max_slider->bounds(min, max);
        max_slider->value(max_slider->maximum());
        max_slider->callback(max_sl, this);
        max_input = new YN_Float_Input(x+120, y, w0, h0, "", max, "%5.1f");
//        max_input->setv(max_slider->maximum());
        max_input->callback(max_in, this);
        end();
    }
    void set_min_max(float in_min, float in_max){
        min = in_min;
        max = in_max;
        min0 = min;
        max0 = max;
        min_input->setv(min);
        min_slider->bounds(min, max);
        min_slider->value(min);
        max_input->setv(max);
        max_slider->bounds(min, max);
        max_slider->value(max);
   }
    void get_min_max(float &out_min, float &out_max){
        out_min = min;
        out_max = max;
    }
protected:
    float min;
    float max;
    float min0;
    float max0;
};

