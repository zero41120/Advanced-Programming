// $Id: shape.cpp,v 1.1 2015-07-16 16:47:51-07 - - $

#include <typeinfo>
#include <unordered_map>
#include <cmath>
using namespace std;

#include "shape.h"
#include "util.h"

static unordered_map<void*,string> fontname {
   {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
   {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
   {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
   {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
   {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
   {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
   {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

static unordered_map<string,void*> fontcode {
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

ostream& operator<< (ostream& out, const vertex& where) {
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

shape::shape() {
   DEBUGF ('c', this);
}

text::text (const string& font, const string& textdata):
      glut_bitmap_font(fontcode[font]), textdata(textdata) {
   DEBUGF ('c', this);
}

ellipse::ellipse (GLfloat width, GLfloat height):
dimension ({width, height}) {
   DEBUGF ('c', this);
}

circle::circle (GLfloat diameter): ellipse (diameter, diameter) {
   DEBUGF ('c', this);
}

polygon::polygon (const vertex_list& vertices): vertices(vertices) {
   DEBUGF ('c', this);
}

rectangle::rectangle (GLfloat width, GLfloat height):
            polygon(
               ([](GLfloat w, GLfloat h) -> vertex_list{
                  GLfloat px = w /2;
                  GLfloat nx = -(w /2);
                  GLfloat py = h /2;
                  GLfloat ny = -(h /2);

                  vertex_list vl;
                  vl.push_back(vertex{nx, py});
                  vl.push_back(vertex{px, py});
                  vl.push_back(vertex{px, ny}); 
                  vl.push_back(vertex{nx, ny});

                  return vl;
               }) // lambda that generates rectangle vertices
            (width, height) // call lambda
            ) {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

square::square (GLfloat width): rectangle (width, width) {
   DEBUGF ('c', this);
}

diamond::diamond (const GLfloat width, const GLfloat height):
   polygon(
      ([](const GLfloat w, const GLfloat h) -> vertex_list{
         GLfloat px = w /2;
         GLfloat nx = -(w /2);
         GLfloat py = h /2;
         GLfloat ny = -(h /2);
         GLfloat z = 0;

         vertex_list vl;
         vl.push_back(vertex{z, py});
         vl.push_back(vertex{nx, z});
         vl.push_back(vertex{z, ny});
         vl.push_back(vertex{px, z});

         return vl;
      }) // lambda that generates diamond vertices
      (width, height) // call lambda
   ){
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

triangle::triangle(const vertex_list& vertices):
      polygon(vertices) {
   DEBUGF ('c', this);
}

right_triangle::right_triangle(const GLfloat width,
                               const GLfloat height):
    triangle(
      ([] (const GLfloat w, const GLfloat h) -> vertex_list {
        GLfloat x0 = - ((2.0/3.0) * w);
        GLfloat y0 = - ((1.0/3.0) * h);
        
        GLfloat x2 = ((1.0/3.0) * w);
        GLfloat y2 = ((2.0/3.0) * h);

        GLfloat x1 = ((1.0/3.0) * w);
        GLfloat y1 = - ((1.0/3.0) * h);
        
        vertex_list vl;
        vl.push_back(vertex{x0, y0});
        vl.push_back(vertex{x2, y2});
        vl.push_back(vertex{x1, y1});

        return vl;
      }) // lambda that generates right triangle vertices
      (width, height) // call Lambda
   ) {
      DEBUGF ('c', this << "(" << width << "," << height << ")");
}

isosceles::isosceles(const GLfloat width,
                     const GLfloat height):
    triangle(
      ([] (const GLfloat w, const GLfloat h) -> vertex_list {

        GLfloat px = w /2;
        GLfloat nx = -(w /2);
        GLfloat py = h /2;
        GLfloat ny = -(h /2);
        GLfloat z = 0;

        vertex_list vl;
        vl.push_back(vertex{z, py});
        vl.push_back(vertex{nx, ny});
        vl.push_back(vertex{px, ny});

        return vl;
      }) // lambda that generates isosceles vertices
      (width, height) // call Lambda
   ) {
         DEBUGF ('c', this << "(" << width << "," << height << ")");
}

equilateral::equilateral(const GLfloat width):
    triangle(
      ([] (const GLfloat w) -> vertex_list {
         GLfloat p = w /2;
         GLfloat n = -(w /2);
         GLfloat z = 0;

         vertex_list vl;
         vl.push_back(vertex{n, n});
         vl.push_back(vertex{z, p});
         vl.push_back(vertex{p, n});
         
         return vl;
      }) // lambda that generates equilateral vertices
      (width) // call Lambda
   ) {
      DEBUGF ('c', this << "(" << width << ")");
}

void text::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   auto font = glut_bitmap_font;
   glColor3ubv(color.ubvec);

   GLfloat xpos = center.xpos;
   GLfloat ypos = center.ypos;

   glRasterPos2f (xpos, ypos);

   for (auto c : textdata) {
      glutBitmapCharacter(font, c);
   }
}

void ellipse::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   glBegin(GL_LINE_LOOP);
      glColor3ubv(color.ubvec);
      float sides = 36.0;
      for(int i = 0; i < sides; i++) {
         float v = 2.0f * M_PI * i / sides;
         float x = dimension.xpos * cosf(v);
         float y = dimension.ypos * sinf(v);
         glVertex2f(x + center.xpos, y + center.ypos);
      }
   glEnd();
}
/*
void circle::draw(const vertex& center, const rgbcolor& color) const {
   DEBUGF('d', this << "(" << center << "," << color << ")");
   ellipse::draw(center, color);
}
*/

void polygon::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   glBegin(GL_LINE_LOOP);
      glColor3ubv(color.ubvec);
      for (vertex v : vertices) {
        glVertex2f(v.xpos + center.xpos, v.ypos + center.ypos);
      }
    glEnd();
}

void shape::show (ostream& out) const {
   out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const {
   shape::show (out);
   out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const {
   shape::show (out);
   out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const {
   shape::show (out);
   out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) {
   obj.show (out);
   return out;
}

