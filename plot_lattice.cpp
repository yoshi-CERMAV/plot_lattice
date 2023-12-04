//
//  plot_lattice.cpp
//  
//
//  Created by Yoshiharu Nishiyama on 24/11/2023.
//

//#include "plot_lattice.hpp"
#include <iostream>
#include <rotator.h>
#include <vector>
using namespace std;

static FILE *gp;
char gnuplot_command[] = "/usr/local/bin/gnuplot -persist";
static float *plot_data;
static float *xyz0;
#define MAINWIN_W 300
#define MAINWIN_H 300
#define MAX_NUM_POINT 1000000
#define NUM_WIDTH 80
static double deg2rad = M_PI/180;

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Round_Button.H>
#include <YN_input.h>
#include <YN_min_max_slider.h>
#include <math.h>
#include "diffraction_data.h"
YN_Angle_Input *rotation_axe_input;
YN_Angle_Input *rotation_speed_input;
YN_Input *camera_length_input;
YN_Input *energy_input;
YN_Input *wavelength_input;
YN_Input *zone1_x_input;
YN_Input *zone1_y_input;
YN_Input *zone1_z_input;
YN_Input *zone2_x_input;
YN_Input *zone2_y_input;
YN_Input *zone2_z_input;

YN_Angle_Input *search_step_input;
YN_Angle_Input *rot_z_input;
YN_Angle_Input *rot_y_input;
YN_Angle_Input *rot_x_input;
YN_Input *cbrange0_input;
YN_Input *cbrange1_input;
YN_Input *radius_input;
YN_Min_Max *min_max;
YN_Int_Input *frame0, *frame1, *frame_step_input;
diffraction_data *diff_dat;
static int iframe0, iframe1;
static int number_of_frames;
static int number_of_data;
static int number_of_selected_data;
static int frame_step;
static int *frame_end;
static float radius, radius2;
static float axe_rot = 154*deg2rad;
static float sr=sin(axe_rot), cr = cos(axe_rot);
static float rot = 0;
static float srot = sin(rot), crot = cos(rot);
static float rotation_speed = 9.09*deg2rad;
static float search_step = 1 * deg2rad;
static float offset_angles[3];
static vector<float> range_plot_data;
static int plot_mode;
static int color_mode;
static int point_count;
static int data_dim = 4;
static int apply_offset = 0;
static char index_filename[256];
static char peak_list_filename[256];
static char center_filename[256];
static float xmin, xmax, ymin, ymax, zmax, zmin;
typedef struct{
    diffraction_data *d0;
    diffraction_data *d1;
    int frame;
} frame_data;
frame_data *frames;
int get_axes(float *in_xyz, int number_of_points, float anglesteps, float *angles);
void set_angles(float *angles);
int refine(float *in_xyz, int number_of_points, float *angles);

void set_filename(const char file[])
{
    strncpy(index_filename, file, 255);
    strncat(index_filename, ".index", 255);
    strncpy(peak_list_filename, file, 255);
    strncat(peak_list_filename, ".dat", 255);
    strncpy(center_filename, file, 255);
    strncat(center_filename, ".center", 255);
}

inline void rotate_z(float x, float y, float *out)
{
    out[0] = cr * x + sr * y;
    out[1] = -sr * x + cr * y;
}

inline void rotate_x(float *x){
    x[2] = x[1] * srot;
    x[1] *= crot;
}

inline void geom_conv(float *ptr, diffraction_data *diff)
{
    rotate_z(diff->x(), diff->y(), ptr);
    rotate_x(ptr);
    ptr[3] = log(diff->Intensity());
 //   cout << ptr[0] <<" "<<ptr[1]<<" "<<ptr[2]<<endl;
}
float frame2angle(int frame)
{
    return frame * rotation_speed*0.01;
}

inline void check_min_max(float &xmin, float &xmax, float &val){
    if(xmin > val) xmin = val;
    if(xmax < val) xmax = val;
};

inline void add_point(float *fptr){
    range_plot_data.push_back(fptr[0]);
    range_plot_data.push_back(fptr[1]);
    range_plot_data.push_back(fptr[2]);
    range_plot_data.push_back(fptr[3]);
}
void plot_range(Fl_Widget *b, void *)
{
    float radius = radius_input->get();
    float themin, themax;
    min_max->get_min_max(themin, themax);
    float *fptr = plot_data;
    range_plot_data.clear();
    for(int i = 0; i < point_count; i++, fptr +=4){
        switch(plot_mode){
            case 0:
                if(fptr[2] > themin && fptr[2] < themax){
                //                    add_point(fptr);
                    range_plot_data.push_back(fptr[0]);
                    range_plot_data.push_back(fptr[1]);
                    range_plot_data.push_back(fptr[3]);
                }
//                if(fptr[2]<zmin)continue;
//                if(fptr[2]>zmax)continue;
                break;
            case 1:
                if(fptr[1] > themin && fptr[1] < themax){
                    range_plot_data.push_back(fptr[0]);
                    range_plot_data.push_back(fptr[2]);
                    range_plot_data.push_back(fptr[3]);
                }
 //               if(fptr[1]<ymin)continue;
//                if(fptr[1]>ymax)continue;
                break;
            case 2:
                if(fptr[0] > themin && fptr[0] < themax){
                    range_plot_data.push_back(fptr[1]);
                    range_plot_data.push_back(fptr[2]);
                    range_plot_data.push_back(fptr[3]);
                }
//                if(fptr[0]<xmin)continue;
//                if(fptr[0]>xmax)continue;
                break;
        }
//                if(fptr[1] > ymin && fptr[1] < ymax){
//                    add_point(fptr);
    }
    int num = range_plot_data.size()/3;
    fprintf(gp, "set xrange [-%3.1f:%3.1f]\n",radius, radius );
    fprintf(gp, "set yrange [-%3.1f:%3.1f]\n",radius, radius );
//    cout << "range plot num "<<num<<endl;
    fputs("plot \"-\" binary format = \"%float%float%float\" record = ", gp);
    fprintf(gp, "%d u 1:2:3 palette \n",num);
    fwrite(reinterpret_cast<char *>(&(range_plot_data[0])), sizeof(float),num*3, gp);
    fflush(gp);
}

void plot(frame_data *f0, frame_data *f1, int step)
{
    int number = f1 - f0;
    number/=step;
    frame_data *last = f0 + number*step;
    float *fptr = plot_data;
    point_count = 0;
    xmax = -10000;
    xmin = 10000;
    ymax = -10000;
    ymin = 10000;
    zmax = -10000;
    zmin = 10000;
    for(frame_data *fr = f0; fr != last; fr+= step){
        rot = frame2angle(fr->frame);
        srot = sin(rot);
        crot = cos(rot);
         for(diffraction_data *ptr = fr->d0; ptr != fr->d1; ptr++){
             if(ptr->r2()<radius2){
                geom_conv(fptr, ptr);
                point_count++;
                fptr += data_dim;
            }
        }
    }
    
//    ofstream fo("test.xyz");
    fptr = plot_data;
    rotator<float> rot;
    fputs("set size square\n", gp);

    if(apply_offset){
        rot.rotz(offset_angles[0]);
        rot.roty(-offset_angles[1]);
        rot.rotx(offset_angles[2]);
        for(int i = 0; i < point_count; i++, fptr += data_dim){
            rot.apply(fptr);
            check_min_max(xmin, xmax, fptr[0]);
            check_min_max(ymin, ymax, fptr[1]);
            check_min_max(zmin, zmax, fptr[2]);
            
            //       fo << fptr[0] <<" "<< fptr[1]<<" "<< fptr[2]<<endl;
        }
        switch(plot_mode){
            case 0:
                min_max->set_min_max(zmin, zmax);
            case 1:
                min_max->set_min_max(ymin, ymax);
            case 2:
                min_max->set_min_max(xmin, xmax);
        }

        fputs("plot \"-\" binary format = \"%float%float%float%float\" record = ", gp);
        switch(plot_mode){
            case 0:
                if(color_mode)fprintf(gp, "%d u 1:2:4 palette \n", point_count);
                else fprintf(gp, "%d u 1:2:3 palette \n", point_count);
                break;
            case 1:
                if(color_mode) fprintf(gp, "%d u 1:3:4 palette \n", point_count);
                else fprintf(gp, "%d u 1:3:2 palette \n", point_count);
                break;
            case 2:
                if(color_mode) fprintf(gp, "%d u 2:3:4 palette \n", point_count);
                else fprintf(gp, "%d u 2:3:1 palette \n", point_count);
                break;
        }
    }else{
        cout <<"point count "<< point_count <<endl;
        fputs("splot \"-\" binary format = \"%float%float%float%float\" record = ", gp);
        fprintf(gp, "%d u 1:2:3:4 palette \n", point_count);
    }
    fwrite(reinterpret_cast<char *>(plot_data), sizeof(float),point_count*4, gp);
    fflush(gp);
}

static void rotation_axe_changed(Fl_Widget *b, void *) {
    axe_rot = rotation_axe_input->get();
    sr=sin(axe_rot), cr = cos(axe_rot);
    plot(frames+iframe0, frames+iframe1, frame_step);
}

static void speed_changed(Fl_Widget *b, void *) {
    rotation_speed =rotation_speed_input->get();
    plot(frames+iframe0, frames+iframe1, frame_step);
}

void iframe_changed(Fl_Widget *w, void *data)
{
    iframe0 = frame0->val();
    iframe1 = frame1->val();
    plot(frames+iframe0, frames+iframe1, frame_step);
}

void frame_step_changed(Fl_Widget *w, void *data)
{
    frame_step = frame_step_input->val();
    plot(frames+iframe0, frames+iframe1, frame_step);
}

void search_step_changed(Fl_Widget *w, void *data)
{
    search_step = search_step_input->get();
}

void refine_orientation(Fl_Widget *w, void *data)
{
    apply_offset=0;
    plot(frames+iframe0, frames+iframe1, frame_step);
    float *ptr = xyz0;
    float *ptr1 = plot_data;
    for(int i = 0; i < point_count; i++, ptr+=3, ptr1+=4){
        ptr[0] = ptr1[0];
        ptr[1] = ptr1[1];
        ptr[2] = ptr1[2];
    }
    refine(xyz0, point_count, offset_angles);
    set_angles(offset_angles);
    apply_offset=1;
}

void find_orientation(Fl_Widget *w, void *data)
{
    apply_offset=0;
    cout <<"entered "<<endl;
    plot(frames+iframe0, frames+iframe1, frame_step);
    int step_a = sizeof(float)*3;
    int step_b = sizeof(float)*4;
//    number_of_selected_data = 0;
    float *ptr = xyz0;
    float *ptr1 = plot_data;
//    ofstream fo("test_xyz");
    cout << xyz0<<endl;
    cout << "going to copy"<<endl;
    for(int i = 0; i < point_count; i++, ptr+=3, ptr1+=4){
//        if (diff_dat[i].r2() < radius2){
        ptr[0] = ptr1[0];
        ptr[1] = ptr1[1];
        ptr[2] = ptr1[2];
//            memcpy(ptr, plot_data+step_b*i, step_a);
//        fo <<ptr[0]<<" "<< ptr[1] <<" "<< ptr[2]<<endl;

 //           ptr += 3;

//            number_of_selected_data++;
//        }
    }
    cout << "searching" <<endl;
    get_axes(xyz0, point_count, search_step, offset_angles);
    set_angles(offset_angles);
    apply_offset = 1;
}

void radius_changed(Fl_Widget *w, void *data)
{
    radius = radius_input->get();
    radius2 = radius*radius;
    float *ptr = xyz0;
    plot(frames+iframe0, frames+iframe1, frame_step);
//    fprintf(gp, "set xra [-%5.2f:%5.2f];\n", radius, radius);
//    fprintf(gp, "set yra [-%5.2f:%5.2f];\n", radius, radius);
//    fprintf(gp, "set zra [-%5.2f:%5.2f];\n", radius, radius);
//    fputs("rep\n", gp);
 //   fflush(gp);
}
void read_diffraction_data()
{
    struct stat st;
    if(stat(peak_list_filename,&st)!=0) {
        cerr<<"file does not exist "<<peak_list_filename<<endl;
    }
    diffraction_data dat;
    number_of_data = st.st_size / sizeof(dat);
    cout << "number of data "<<number_of_data<<endl;
    if(stat(index_filename,&st)!=0) {
        cerr<<"file does not exist "<<index_filename<<endl;
    }
    number_of_frames = st.st_size/sizeof(int);
    cout << "number of frames "<<number_of_frames << endl;
    if(frame_end) delete [] frame_end;
    frame_end = new int[number_of_frames];
    ifstream fi_index(index_filename);
    fi_index.read(reinterpret_cast<char *>(frame_end), st.st_size);
    frames = new frame_data[number_of_frames];
    if(diff_dat) delete[] diff_dat;
    diff_dat = new diffraction_data [number_of_data];
    ifstream fi(peak_list_filename);
    cout << "size of (diff_dat) "<< sizeof(diff_dat[0])<<endl;
    fi.read(reinterpret_cast<char *>(diff_dat), sizeof(diff_dat[0])*number_of_data);
    frames[0].d0 = diff_dat;
    for(int i = 0; i < number_of_frames; i++) {
        frames[i].d1 = diff_dat+frame_end[i];
        if(i) frames[i].d0 = frames[i-1].d1;
        frames[i].frame = i;
    }
//    for(int i = 0; i < number_of_frames; i++){
 //       cout << i <<" "<<frame_end[i]<<" "<<frames[i].d0 <<" "<<frames[i].d1<<endl;
 //   }
    for(int i = 0; i< 10; i++){
        cout << i<<"\t";
        diff_dat[i].dump();
    }
}

void set_angles(float *a)
{
    rot_z_input->setv(a[0]);
    rot_y_input->setv(a[1]);
    rot_x_input->setv(a[2]);
}

static void button_cb(Fl_Button *b, void *) {
    if(! strncmp(b->label(), "xy", 2)) plot_mode = 0;
    if(! strncmp(b->label(), "xz", 2)) plot_mode = 1;
    if(! strncmp(b->label(), "yz", 2)) plot_mode = 2;
    plot(frames+iframe0, frames+iframe1, frame_step);
}
static void button_color(Fl_Button *b, void *) {
    if(! strncmp(b->label(), "depth", 2)) color_mode = 0;
    if(! strncmp(b->label(), "intensity", 2)) color_mode = 1;
    plot(frames+iframe0, frames+iframe1, frame_step);
}
static void set_zrange(Fl_Button *b, void *) {
    if(! strncmp(b->label(), "depth", 2)) color_mode = 0;
    if(! strncmp(b->label(), "intensity", 2)) color_mode = 1;
    plot(frames+iframe0, frames+iframe1, frame_step);
}

int main(int argc, char *argv[])
{
    diff_dat = NULL;
    frame_end = NULL;
    gp = popen(gnuplot_command, "w");
    fputs("set view equal xyz;\n set ticslevel 0;\n", gp);
    fflush(gp);
    plot_data = new float[MAX_NUM_POINT*4];
    xyz0 = new float[MAX_NUM_POINT*3];
    range_plot_data.reserve(1000);
    set_filename(argv[1]);
    read_diffraction_data();
    for(int i = 0; i < 10; i++){
        diff_dat[i].dump();
    }
    radius = 250;
    radius2 = 250*250;
    int hpos = 100;
    int vpos = 30;
    Fl_Window window(400, 500, "diffraction frame");
    window.begin();
    
    rotation_axe_input = new YN_Angle_Input(hpos, vpos , NUM_WIDTH, 25, "rotation axis", axe_rot);
    rotation_axe_input-> callback((Fl_Callback*) rotation_axe_changed);
    vpos +=30;
    rotation_speed_input = new YN_Angle_Input(hpos, vpos, NUM_WIDTH, 25, "rotation_speed",rotation_speed);
    rotation_speed_input -> callback((Fl_Callback*) speed_changed);
    vpos +=30;
    frame0 = new YN_Int_Input(hpos, vpos, NUM_WIDTH, 25, "1st frame", 0);
    vpos +=30;
    iframe0 = 0;
    iframe1 = number_of_frames;
    frame1 = new YN_Int_Input(hpos, vpos, NUM_WIDTH, 25, "last frame", number_of_frames);
    frame0->callback((Fl_Callback*) iframe_changed);
    frame1->callback((Fl_Callback*) iframe_changed);
    vpos +=30;
    frame_step = 1;
    frame_step_input = new YN_Int_Input(hpos, vpos, NUM_WIDTH, 25, "frame step", 1);
    frame_step_input->callback((Fl_Callback*) frame_step_changed);
    vpos +=30;
    radius_input = new YN_Input(hpos, vpos, NUM_WIDTH, 25, "radius",250);
    radius_input -> callback((Fl_Callback*) radius_changed);
    vpos +=30;
    search_step_input = new YN_Angle_Input(hpos, vpos, NUM_WIDTH, 25, "search step", search_step);
    search_step_input -> callback((Fl_Callback*)search_step_changed);
    vpos +=30;
    Fl_Button *b5 = new Fl_Button(hpos, vpos, NUM_WIDTH, 25, "find axes");
    b5->callback((Fl_Callback *)find_orientation);
    vpos +=30;
    //    camera_length_input = new YN_Input(hpos, vpos, 50, 25, "radius",250);
    //    radius_input -> callback((Fl_Callback*) radius_changed);
    //    YN_Input *camera_length_input;
    //    YN_Input *energy_input;
    //    YN_Input *wavelength_input;
    rot_z_input =  new YN_Angle_Input(hpos, vpos, NUM_WIDTH, 25, "rot_z",0, "%5.4f");
    vpos +=30;
    rot_y_input =  new YN_Angle_Input(hpos, vpos, NUM_WIDTH, 25, "rot_y",0, "%5.4f");
    vpos +=30;
    rot_x_input = new YN_Angle_Input(hpos, vpos, NUM_WIDTH, 25, "rot_x",0, "%5.4f");
    vpos +=30;
    Fl_Button *b6 = new Fl_Button(hpos, vpos, NUM_WIDTH, 25, "refine");
    b6->callback((Fl_Callback *)refine_orientation);
    vpos +=40;
    min_max = new YN_Min_Max(hpos, vpos, 250, 50, "minmax", -10, 100);
    min_max->callback((Fl_Callback *) plot_range);
    hpos = 250;
    vpos = 30;
    {
        Fl_Group* o = new Fl_Group(hpos, vpos,70, 90, "chose display");
        o->box(FL_THIN_UP_FRAME);
        o->begin();
        { Fl_Round_Button* o = new Fl_Round_Button(hpos, vpos, 70, 25, "xy");
            vpos +=30;
            o->tooltip("showing xy projection.");
            o->type(102);
            o->set();
            o->down_box(FL_ROUND_DOWN_BOX);
            o->callback((Fl_Callback*) button_cb);
        } // Fl_Round_Button* o
        { Fl_Round_Button* o = new Fl_Round_Button(hpos, vpos, 70, 25, "xz");
            vpos +=30;
            o->tooltip("showing xz projection.");
            o->type(102);
            o->down_box(FL_ROUND_DOWN_BOX);
            o->callback((Fl_Callback*) button_cb);
        }
        { Fl_Round_Button* o = new Fl_Round_Button(hpos, vpos, 70, 25, "yz");
            vpos +=30;
            o->tooltip("showing yz projection.");
            o->type(102);
            o->down_box(FL_ROUND_DOWN_BOX);
            o->callback((Fl_Callback*) button_cb);
        }
        o->end();
    } vpos+=30;

    {
        Fl_Group* o = new Fl_Group(hpos, vpos,70, 70, "color code");
        o->box(FL_THIN_UP_FRAME);
        o->begin();
        { Fl_Round_Button* o = new Fl_Round_Button(hpos, vpos, 70, 25, "depth");
            vpos +=30;
            o->tooltip("depth");
            o->type(102);
            o->set();
            o->down_box(FL_ROUND_DOWN_BOX);
            o->callback((Fl_Callback*) button_color);
        } // Fl_Round_Button* o
        { Fl_Round_Button* o = new Fl_Round_Button(hpos, vpos, 70, 25, "intensity");
            vpos +=30;
            o->tooltip("intensity");
            o->type(102);
 //           o->set();
            o->down_box(FL_ROUND_DOWN_BOX);
            o->callback((Fl_Callback*) button_color);
        }
        o->end();
    }

    window.end();
    window.show(argc, argv);
    return Fl::run();

}
