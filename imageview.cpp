#include <iostream>
#include <cairo.h>	    // 绘图所需要的头文件
#include <gtk/gtk.h>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"

#define SCALE_SIZE_MIN 100


struct ImageViewData{
    bool left_button_pressed;
    bool mid_button_pressed;
    bool right_button_pressed;

    bool limit_movement;


    gdouble left_start_pos_x;
    gdouble left_start_pos_y;

    gint scale_up;
    gint scale_down;

    gint image_position_x;    //image position
    gint image_position_y;
    gint image_width;
    gint image_height;
    gint image_scaled_width;
    gint image_scaled_height;

    GdkPixbuf *pixbuf;

    guint last_key_value;

};

// event wrapper
void imageview_mouse_press(bool left,bool mid,bool right,int x,int y,ImageViewData *view_data){
    if(left){
        view_data->left_button_pressed = true;
        view_data->left_start_pos_x = x;
        view_data->left_start_pos_y = y;
    }
    if(mid)view_data->mid_button_pressed = true;
    if(right)view_data->right_button_pressed = true;
}
void imageview_mouse_release(bool left,bool mid,bool right,ImageViewData *view_data){
    if(left)view_data->left_button_pressed = false;
    if(mid)view_data->mid_button_pressed = false;
    if(right)view_data->right_button_pressed = false;
}
bool imageview_mouse_move(int x,int y,int win_width,int win_height,ImageViewData *view_data){
    if(view_data->left_button_pressed){
        // image size
        gint image_width = view_data->image_scaled_width;
        gint image_height = view_data->image_scaled_height;

        // image position
        gdouble old_pos_x = view_data->image_position_x;
        gdouble old_pos_y = view_data->image_position_y;
        gdouble new_pos_x = old_pos_x + x - view_data->left_start_pos_x;
        gdouble new_pos_y = old_pos_y + y - view_data->left_start_pos_y;

        gdouble new_end_x = new_pos_x + image_width;
        gdouble new_end_y = new_pos_y + image_height;
        gdouble old_end_x = old_pos_x + image_width;
        gdouble old_end_y = old_pos_y + image_height;

        // mouse movement limit
        if(view_data->limit_movement){

            if(image_width > win_width){
                if(old_pos_x <= 0 && new_pos_x > 0)new_pos_x = 0;
                if(old_end_x >= win_width && new_end_x < win_width)new_pos_x = win_width - image_width;
            }
            else{
                if(old_pos_x >= 0 && new_pos_x < 0)new_pos_x = 0;
                if(old_end_x <= win_width && new_end_x > win_width)new_pos_x = win_width - image_width;
            }

            if(image_height > win_height){
                if(old_pos_y <= 0 && new_pos_y > 0)new_pos_y = 0;
                if(old_end_y >= win_height && new_end_y < win_height)new_pos_y = win_height - image_height;
            }
            else{
                if(old_pos_y >=0 && new_pos_y < 0)new_pos_y = 0;
                if(old_end_y <= win_height && new_end_y > win_height)new_pos_y = win_height - image_height;
            }
        }

        // update position
        view_data->image_position_x = new_pos_x;
        view_data->image_position_y = new_pos_y;
        view_data->left_start_pos_x = x;
        view_data->left_start_pos_y = y;
        return true;
    }
    return false;
}
bool imageview_wheel_scroll(int x,int y,bool scroll_up,ImageViewData *view_data){
    gdouble img_pos_x = view_data->image_position_x;
    gdouble img_pos_y = view_data->image_position_y;
    gint scale_up = view_data->scale_up;
    gint scale_down = view_data->scale_down;

    gdouble pointer_x = x;
    gdouble pointer_y = y;

    gdouble pointer_offset_x = pointer_x - img_pos_x;
    gdouble pointer_offset_y = pointer_y - img_pos_y;

    if(scroll_up) {

        img_pos_x = img_pos_x - pointer_offset_x;
        img_pos_y = img_pos_y - pointer_offset_y;
        if(scale_down == 0){                //scale up
            scale_up++;
        } else{                             //scale up to origin
            scale_down --;
            if(scale_down == 0){            //origin
                scale_up = 1;
            }
        }
    }else{

        img_pos_x = img_pos_x + pointer_offset_x/2;;
        img_pos_y = img_pos_y + pointer_offset_y/2;
        if(scale_up == 1 || scale_up == 0){   //scale down
            if(scale_up== 1)scale_up = 0; // scale_down=0;
            scale_down++;
        }
        else{                                   //scale down to origin
            scale_up--;
        }
    }

    gint scaled_width,scaled_height;
    bool scaled_valid = true;
    if(scale_down == 0){
        scaled_width = view_data->image_width << (scale_up - 1);
        scaled_height = view_data->image_height << (scale_up - 1);
    }else{
        scaled_width = view_data->image_width >> scale_down;
        scaled_height = view_data->image_height >> scale_down;
        if(scaled_width < SCALE_SIZE_MIN || scaled_height < SCALE_SIZE_MIN) scaled_valid = false;
    }

    if(scaled_valid){
        view_data->scale_down = scale_down;
        view_data->scale_up = scale_up;
        view_data->image_scaled_width = scaled_width;
        view_data->image_scaled_height = scaled_height;
        view_data->image_position_x = img_pos_x;
        view_data->image_position_y = img_pos_y;
        return true;
    }
    return false;
}



// gtk event
gboolean on_window_mouse_press(GtkWidget *widget,GdkEventButton *event,gpointer data) {
    if(event->button >= 1 && event->button <= 3){
        bool left = event->button == 1;
        bool mid = event->button == 2;
        bool right = event->button == 3;
        imageview_mouse_press(left,mid,right,(int)event->x,(int)event->y,(ImageViewData*)data);
    }
    return FALSE;
}
gboolean on_window_mouse_release(GtkWidget *widget,GdkEventButton *event,gpointer data) {
    if(event->button >= 1 && event->button <= 3) {
        bool left = event->button == 1;
      bool mid = event->button == 2;
        bool right = event->button == 3;
        imageview_mouse_release(left,mid,right,(ImageViewData*)data);
    }
    return FALSE;
}
gboolean on_window_mouse_move(GtkWidget *widget,GdkEventMotion *event,gpointer data){

    gint win_width,win_height;
    gtk_window_get_size(GTK_WINDOW(widget),&win_width,&win_height);

    bool res = imageview_mouse_move((int)event->x,(int)event->y,win_width,win_height,(ImageViewData*)data);
    // redraw
    if(res) gtk_widget_queue_draw(widget);

    return FALSE;
}
gboolean on_window_wheel_scroll(GtkWidget *widget,GdkEventScroll *event,gpointer data){
    bool res = imageview_wheel_scroll((int)event->x,(int)event->y,event->direction == GdkScrollDirection::GDK_SCROLL_UP,(ImageViewData*)data);
    if(res) gtk_widget_queue_draw(widget);
    return FALSE;
}
gboolean on_window_key_press(GtkWidget *widget,GdkEventKey  *event,gpointer data){
    ((ImageViewData*)data)->last_key_value = event->keyval;
}


// 绘图事件
// TODO: paint event wrapper
gboolean on_expose_event (GtkWidget * widget, GdkEventExpose *event, gpointer data)
{
    ImageViewData *view_data = (ImageViewData*)data;
    cairo_t *cr = gdk_cairo_create(widget->window);
    GdkPixbuf* dst_pixbuf = NULL;

    // image size
    int scaled_image_width = view_data->image_scaled_width;
    int scaled_image_height = view_data->image_scaled_height;

    // window size
    gint win_width,win_height;
    gtk_window_get_size(GTK_WINDOW(widget),&win_width,&win_height);

    // image position in window
    int img_pos_start_x = view_data->image_position_x;
    int img_pos_start_y = view_data->image_position_y;
    int img_pos_end_x = img_pos_start_x + scaled_image_width;
    int img_pos_end_y = img_pos_start_y + scaled_image_height;


    if(view_data->scale_down > 0){      //scale down
        dst_pixbuf = gdk_pixbuf_scale_simple(view_data->pixbuf,
                                             scaled_image_width,
                                             scaled_image_height,
                                             GDK_INTERP_BILINEAR);
    }else if(view_data->scale_up >1){   //scale up

        if(img_pos_start_x >= win_width || img_pos_start_y >= win_height || img_pos_end_x <= 0 || img_pos_end_y <= 0){
            // in the window turn to black
            dst_pixbuf = gdk_pixbuf_new_subpixbuf(view_data->pixbuf, 0, 0,1,1);
            img_pos_start_x = win_width;
            img_pos_start_y = win_height;

        }else {

            // coordinate in image
            int scaled_img_show_start_x, scaled_img_show_start_y;
            int scaled_img_show_end_x, scaled_img_show_end_y;
            int scaled_factor = view_data->scale_up - 1;

            // x
            if (img_pos_start_x >= 0) scaled_img_show_start_x = 0;
            else scaled_img_show_start_x = -img_pos_start_x;

            if (img_pos_end_x <= win_width) scaled_img_show_end_x = scaled_image_width;
            else scaled_img_show_end_x = scaled_image_width - (img_pos_end_x - win_width);


            // y
            if (img_pos_start_y >= 0) scaled_img_show_start_y = 0;
            else scaled_img_show_start_y = -img_pos_start_y;

            if (img_pos_end_y <= win_height) scaled_img_show_end_y = scaled_image_height;
            else scaled_img_show_end_y = scaled_image_height - (img_pos_end_y - win_height);

            // clip cooinate in origin image
            int clip_start_x = scaled_img_show_start_x >> scaled_factor;
            int clip_start_y = scaled_img_show_start_y >> scaled_factor;

            int clip_end_x = (scaled_img_show_end_x + (1 << scaled_factor) - 1) >> scaled_factor;
            int clip_end_y = (scaled_img_show_end_y + (1 << scaled_factor) - 1) >> scaled_factor;

            int clip_width = clip_end_x - clip_start_x;
            int clip_height = clip_end_y - clip_start_y;

            // sub pixbuf
            GdkPixbuf *clip_pixbuf = gdk_pixbuf_new_subpixbuf(view_data->pixbuf, clip_start_x, clip_start_y,
                                                              clip_width,
                                                              clip_height);

            //
            int clip_scaled_width = (clip_width) << scaled_factor;
            int clip_scaled_height = (clip_height) << scaled_factor;

            // scale
            GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(clip_pixbuf,
                                                               clip_scaled_width,
                                                               clip_scaled_height,
                                                               GDK_INTERP_NEAREST);

            // final image
            clip_start_x = scaled_img_show_start_x - (clip_start_x << scaled_factor);
            clip_start_y = scaled_img_show_start_y - (clip_start_y << scaled_factor);

            clip_end_x = clip_scaled_width - ((clip_end_x << scaled_factor) - scaled_img_show_end_x);
            clip_end_y = clip_scaled_height - ((clip_end_y << scaled_factor) - scaled_img_show_end_y);

            clip_width = clip_end_x - clip_start_x;
            clip_height = clip_end_y - clip_start_y;

            dst_pixbuf = gdk_pixbuf_new_subpixbuf(scaled_pixbuf, clip_start_x, clip_start_y,
                                                  clip_width,
                                                  clip_height);

            if (img_pos_start_x < 0)img_pos_start_x = 0;
            if (img_pos_start_y < 0)img_pos_start_y = 0;


            g_object_unref(clip_pixbuf);
            g_object_unref(scaled_pixbuf);
        }
    }else{                              //origin
        dst_pixbuf = view_data->pixbuf;
    }

    if(dst_pixbuf != NULL)
        gdk_cairo_set_source_pixbuf(cr, dst_pixbuf, img_pos_start_x, img_pos_start_y);

    cairo_paint(cr);	// draw
    if(dst_pixbuf != view_data->pixbuf && dst_pixbuf != NULL) g_object_unref(dst_pixbuf);



    cairo_destroy(cr);	// 回收所有Cairo环境所占用的内存资源

    return FALSE;// 必须返回FALSE
}



int imageview_show(const char *window_name,const cv::Mat &img){

    cv::Mat img_rgb;
    if(img.type() == CV_8UC1){
        cv::cvtColor(img,img_rgb,cv::COLOR_GRAY2RGB);
    }else if(img.type() == CV_8UC3){
        cv::cvtColor(img,img_rgb,cv::COLOR_BGR2RGB);
    }else{
        return 0; // unvalid
    }
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(img_rgb.data,GdkColorspace::GDK_COLORSPACE_RGB,
                                                 0,8,img_rgb.cols,img_rgb.rows,
                                                 img_rgb.step1(0),NULL,NULL);


    gtk_init (0, NULL);

    // global image view data
    ImageViewData image_view_data;
    image_view_data.left_button_pressed = false;
    image_view_data.mid_button_pressed = false;
    image_view_data.right_button_pressed = false;
    image_view_data.limit_movement = true;
    image_view_data.image_position_x = 0;
    image_view_data.image_position_y = 0;
    image_view_data.last_key_value = 0;


    image_view_data.pixbuf = pixbuf;
//    image_view_data.pixbuf = gdk_pixbuf_new_from_file(filepath, NULL);
    image_view_data.image_width = gdk_pixbuf_get_width(image_view_data.pixbuf);
    image_view_data.image_height = gdk_pixbuf_get_height(image_view_data.pixbuf);

    image_view_data.scale_up = 1;
    image_view_data.scale_down = 0;
    image_view_data.image_scaled_width = image_view_data.image_width;
    image_view_data.image_scaled_height = image_view_data.image_height;


    // setup window
    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL); // 顶层窗口
    g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);	// 中央位置显示
    gtk_widget_set_size_request(window, 640, 480);		            // 窗口最小大小
    gtk_widget_set_app_paintable(window, TRUE);	                                // 允许窗口可以绘图

    // 绘图事件信号与回调函数的连接
    gint event_mask = 0;
    event_mask |= GDK_POINTER_MOTION_MASK;
    event_mask |= GDK_BUTTON_PRESS_MASK;
    event_mask |= GDK_BUTTON_RELEASE_MASK;
    event_mask |= GDK_SCROLL_MASK;
    event_mask |= GDK_KEY_PRESS_MASK;

    gtk_widget_add_events(window,event_mask);
    g_signal_connect(window, "expose-event", G_CALLBACK(on_expose_event), &image_view_data);
    g_signal_connect(window,"button-release-event",G_CALLBACK(on_window_mouse_release),&image_view_data);
    g_signal_connect(window,"button-press-event",G_CALLBACK(on_window_mouse_press),&image_view_data);
    g_signal_connect(window,"motion-notify-event",G_CALLBACK(on_window_mouse_move),&image_view_data);
    g_signal_connect(window,"scroll-event",G_CALLBACK(on_window_wheel_scroll),&image_view_data);
    g_signal_connect(window,"key-press-event",G_CALLBACK(on_window_key_press),&image_view_data);

    gtk_widget_show_all(window);	// 显示所有控件
    gtk_main();
    g_object_unref(image_view_data.pixbuf);   //release??

    return image_view_data.last_key_value;
}