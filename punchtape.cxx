// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

// Facit N4000 Punch Tape Simulator
//
// v1.00 07/19/2025 erco@seriss.com Initial version
//
//     Only handles default raw mode for punching 8-channel tapes.
//

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>	// malloc
#include <string.h>	// memset

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/fl_draw.H>

#define HOLE_SIZE     12	// diameter of punch holes (in pixels)
#define HOLE_SPACING  3
#define SPROCKET_SIZE 5		// diameter of sprocket holes (in pixels)
#define SLIDER_HEIGHT 14	// height of horizontal slider

//    TAPE
//    ___________________
//
//       O O O O O O O       <- 0x80
//       O O O O O O O       <- 0x40
//       O O O O O O O       <- 0x20
//       O O O O O O O       <- 0x10
//       O O O O O O O       <- 0x08
//       . . . . . . .       <- sprockets
//       O O O O O O O       <- 0x04
//       O O O O O O O       <- 0x02
//       O O O O O O O       <- 0x01
//    ____________________
// 

// Facit simulator class widget
class FacitCanvas : public Fl_Widget {
    int datasize;
    unsigned char *data;
    int tapewidth;
    int xoffset;            // offset for drawing tape (used for scrolling)
    Fl_Slider *slider;      // (on resize, adjusts slider bounds for scrolling)
public:
    // CTOR
    FacitCanvas(int X,int Y,int W,int H,const char*L=0) : Fl_Widget(X,Y,W,H,L) {
        // Private data members
        datasize = 0;
        data = 0;
        tapewidth = 0;
        xoffset = 0;
        slider = 0;
        // Widget init
        color(0x00cdff00);  // light blue tape
    }

    // Set the raw data for the facit tape
    //    new_datasize  - size in bytes of the data buffer
    //    new_data      - the raw facit data bytes buffer
    //    new_tapewidth - length of tape including end margins (width)
    //
    void SetData(int            new_datasize,
                 unsigned char* new_data,
                 int            new_tapewidth) {
        datasize  = new_datasize;
        data      = new_data;
        tapewidth = new_tapewidth;
    }

    // Set X offset for scrolling thru the tape
    void SetXOffset(int val) {
        xoffset = val;
        redraw();
    }

    // Get X offset
    int GetXOffset(void) {
        return xoffset;
    }

    // Set the slider widget
    //    The class uses this to update the slider's bounds() when window resized
    //
    void SetSlider(Fl_Slider *w) {
        slider = w;
    }

    // Draw the facit widget
    void draw() {
        // DRAW BG
        fl_color(color());
        fl_rectf(x(),y(),w(),h());

        // DRAW TAPE'S PUNCH HOLES
        unsigned char *p = data;
        int start_x = 10 - xoffset;	        // starting x offset from left side of tape
        int start_y = 10;		        // starting y offset from top of tape
        int px = start_x;		        // running punch hole x position
        for ( int t=0; t<datasize; t++ ) {	// loop thru raw facit data
            // Draw holes in this tape column
            int py = start_y;		        // start at top of tape and walk down thru bits
            if ( px >= 0 && px <= w() ) {       // only draw if within xywh of window
                for ( int bit=0x80; (bit & 0x00ff); bit >>= 1 ) {	// shift thru bits 0x80 .. 0x01

                    // Draw a punch hole
                    if ( *p & bit ) {
                        // Filled circles
                        fl_color(FL_BLACK);
                        fl_begin_polygon();
                        fl_circle(px, py, HOLE_SIZE/2);
                        fl_end_polygon();
                    } else {
                        // Empty circles
                        fl_color(0x00addf00);  // slightly darker than bg
                        fl_begin_line();
                        fl_circle(px, py, HOLE_SIZE/2);
                        fl_end_line();
                    }
                    py += HOLE_SIZE + HOLE_SPACING;

                    // Handle drawing the sprocket row
                    if (bit == 0x08) {
                        fl_color(FL_BLACK);
                        fl_begin_polygon();
                        fl_circle(px, py, SPROCKET_SIZE/2);
                        fl_end_polygon();
                        py += (HOLE_SIZE + HOLE_SPACING);
                    }
                }
            }
            px += HOLE_SIZE + HOLE_SPACING; // move down the tape
            p++;                            // next data byte
        }
    }

    // Override window resizes
    void resize(int X,int Y,int W,int H) {
        Fl_Widget::resize(X,Y,W,H);         // let window handle resize
        // If a scroller (Fl_Sldier) is defined, update its bounds with the new size
        if (slider) {
            slider->bounds(0, tapewidth - W);
            // Clamp slider's value to be within window
            if (slider->value() > slider->maximum()) {
                slider->value(slider->maximum());
            }
            slider->redraw();
        }
    }
};

// Return file size in bytes
int FileSize(const char *filename) {
    struct stat buf;
    memset(&buf, 0, sizeof(struct stat));
    if (stat(filename, &buf)) {
        perror(filename);
	exit(1);
    }
    return (int)(buf.st_size);
}

// READ RAW DATA INTO BUFFER, RETURN #BYTES READ
unsigned char* ReadRawData(const char *filename, int &size) {
    size = FileSize(filename);
    unsigned char *data = (unsigned char*)malloc(size);
    FILE *fp = fopen(filename, "r");
    if (!fp) { perror(filename); exit(1); }
    fread(data, 1, size, fp);
    fclose(fp);
    return data;
}

// Handle slider (scrollbar) movement
void Slider_CB(Fl_Widget *w, void *userdata) {
    Fl_Slider *slider = (Fl_Slider*)w;
    FacitCanvas *facit = (FacitCanvas*)userdata;
    // printf("slider=%d, ", (int)slider->value());
    // printf("facit xoff=%d\n", facit->GetXOffset());
    facit->SetXOffset(slider->value());
}

// Main program
int main(int argc, char *argv[]) {

    // Handle command line arguments first
    if ( argc != 2 ) {
        printf("usage: %s punchdata.bin   - raw binary data to send to a Facit N4000\n", argv[0]);
        return 0;
    }

    // Read user's raw facit data into a buffer
    //     This buffer will be used to draw, and redraw the tape
    //
    int size = 0;
    unsigned char *buf = ReadRawData(argv[1], size);
    // Determine how long the tape needs to be
    int tapewidth = (size*(HOLE_SIZE+HOLE_SPACING) + (50*2));	// tape length in pixels
    int tapeheight = (HOLE_SIZE*8)+(HOLE_SPACING*8) + 20;       // tape height (including margins)

    // Create GUI, assign raw buffer to Facit class simulator
    Fl_Double_Window win(1200,tapeheight+SLIDER_HEIGHT,"Facit N4000 Punch Simulator");
    FacitCanvas facit(0,0,win.w(),tapeheight);
    facit.SetData(size, buf, tapewidth);		// assign raw data to facit simulator

    // Create a slider to "scroll" (xoffset) the drawing position for the tape
    Fl_Slider slider(0,facit.h(),win.w(),SLIDER_HEIGHT);
    facit.SetSlider(&slider);
    slider.type(1);                          // horiz slider
    slider.value(0);
    slider.bounds(0, tapewidth - win.w());
    slider.slider_size(10.0/100.0);          // slider's 'tab' size as fractional size of widget
    slider.callback(Slider_CB, (void*)&facit);
    // End the window definition, and show() it
    win.end();
    win.resizable(facit);
    win.show();
    return(Fl::run());
}
