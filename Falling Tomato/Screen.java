package com.example.yeyette.assign5;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.view.View;

/**
 * Created by Yeyette on 5/24/2015.
 */
public class Screen extends View{

    Tomato tomato = new Tomato();
    public double difX, difY, drainX, drainY, drainR = 120, dif;
    float accelx = 0, accely = 0;
    float h, w;

    public Screen(Context context) {
        super(context);
    }

    /********* Runs all methods for the game *********/
    public Runnable animator = new Runnable() {
        @Override
        public void run() {
            physics();
            invalidate();
            postDelayed(this, 20); // 20 is in milliseconds
        }
    };

    /********* Draws all of the components on the screen/canvas *********/
    @Override
    protected void onDraw(Canvas c) {
        super.onDraw(c);
        h = c.getHeight();
        w = c.getWidth();

        Paint paint = new Paint();

        /********* Draw background *********/
        paint.setColor(Color.WHITE);
        c.drawRect(0,0,c.getWidth(),c.getHeight(), paint);

        /********* Draw walls *********/
        paint.setColor(Color.LTGRAY);
        paint.setStrokeWidth(25f);
        c.drawLine(0, h/3, w*2/3, h/3, paint);

        paint.setColor(Color.LTGRAY);
        paint.setStrokeWidth(25f);
        c.drawLine(w/3, h*2/3, w, h*2/3, paint);

        /********* Draw drain *********/
        paint.setColor(Color.DKGRAY);
        drainX = w/2;
        drainY = (h-(h/9));
        c.drawCircle((float)drainX, (float)drainY,(float)drainR, paint);

        /********* Draw tomato *********/
        paint.setColor(Color.RED);
        c.drawCircle(tomato.x, tomato.y, tomato.r, paint);
    }

    /********* Calculations for the tomato hitting the boundaries *********/
    public void physics() {

        /* Velocity should be affected by the acceleration */
        tomato.velx += accelx*0.08;
        tomato.vely += accely*0.08;

        /* Every tick and position should be affected by the velocity */
        tomato.x += tomato.velx*1.2;
        tomato.y += tomato.vely*1.2;

        difX = tomato.x - drainX;
        difY = tomato.y - drainY;

        dif = Math.sqrt((difX)*(difX) + (difY)*(difY));

        /***** Tomato in the drain - WIN! ******/
        if (dif < (tomato.r + drainR)) {
            tomato.x = w/16;
            tomato.y = h/16;
        }

        /****** Top Border Boundary ******/
        if ((tomato.y - tomato.r) < 0) {
            tomato.vely = tomato.vely*-(float).5;
            tomato.y = tomato.r;
        }

        /****** Bottom Border Boundary ******/
        if ((tomato.y + tomato.r) > h) {
            tomato.vely = tomato.vely*-(float).5;
            tomato.y = h - tomato.r;
        }

        /****** Left Border Boundary ******/
        if ((tomato.x - tomato.r) < 0) {
            tomato.velx = tomato.velx*-(float).5;
            tomato.x = tomato.r;
        }

        /****** Right Border Boundary ******/
        if ((tomato.x + tomato.r) > w) {
            tomato.velx = tomato.velx*-(float).5;
            tomato.x = w - tomato.r;
        }

        /****** Wall Boundaries ******/
        float firstHeight = h/3.0f;
        float firstWidth = w*2.0f/3.0f;

        float secondHeight = h*2.0f/3.0f;
        float secondWidth = w/3.0f;

        /* First wall */
        if (((tomato.y + tomato.r) > firstHeight) && ((tomato.y - tomato.r) < firstHeight)){
            if (tomato.x > firstWidth) {
            }
            else if (tomato.y < firstHeight) {
                tomato.y = firstHeight - tomato.r;
                tomato.vely = tomato.vely*-(float).5;
            }
            else if (tomato.y > firstHeight) {
                tomato.y = firstHeight + tomato.r;
                tomato.vely = tomato.vely*-(float).5;
            }
        }

        /* Second wall */
        if (((tomato.y + tomato.r) > secondHeight) && ((tomato.y - tomato.r) < secondHeight)){
            if (tomato.x < secondWidth) {
            }
            else if (tomato.y < secondHeight) {
                tomato.y = secondHeight - tomato.r;
                tomato.vely = tomato.vely*-(float).5;
            }
            else if (tomato.y > secondHeight) {
                tomato.y = secondHeight + tomato.r;
                tomato.vely = tomato.vely*-(float).5;
            }
        }

    }
}
