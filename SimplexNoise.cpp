//
// Created by c3042750 on 09/11/2023.
//

#include "SimplexNoise.h"
// Credit to https://gist.github.com/Flafla2/f0260a861be0ebdeef76

Noise::Noise(int width, int height) {
    p = new int[512];

    for(int x=0;x<512;x++) {
        p[x] = hashThing[x%256];
    }

    texture = CreateTexture(width, height);
}

double Noise::GetNoise(double x, double y, double z, int octaves, double persistence) {
    double total = 0;
    double frequency = 1;
    double amplitude = 1;
    double maxValue = 0;  // Used for normalizing result to 0.0 - 1.0
    for(int i=0;i<octaves;i++) {
        total += GenNoise(x * frequency, y * frequency, z * frequency) * amplitude;

        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= 2;
    }

    return total/maxValue;
}

Noise::~Noise() {
    delete p;
}

double Noise::GenNoise(double x, double y, double z) {
    if(repeat > 0) {									// If we have any repeat on, change the coordinates to their "local" repetitions
        x = (int)x % repeat;
        y = (int)y % repeat;
        z = (int)z % repeat;
    }

    int xi = (int)x & 255;								// Calculate the "unit cube" that the point asked will be located in
    int yi = (int)y & 255;								// The left bound is ( |_x_|,|_y_|,|_z_| ) and the right bound is that
    int zi = (int)z & 255;								// plus 1.  Next we calculate the location (from 0.0 to 1.0) in that cube.
    double xf = x-(int)x;								// We also fade the location to smooth the result.
    double yf = y-(int)y;
    double zf = z-(int)z;
    double u = fade(xf);
    double v = fade(yf);
    double w = fade(zf);

    int aaa, aba, aab, abb, baa, bba, bab, bbb;
    aaa = p[p[p[    xi ]+    yi ]+    zi ];
    aba = p[p[p[    xi ]+inc(yi)]+    zi ];
    aab = p[p[p[    xi ]+    yi ]+inc(zi)];
    abb = p[p[p[    xi ]+inc(yi)]+inc(zi)];
    baa = p[p[p[inc(xi)]+    yi ]+    zi ];
    bba = p[p[p[inc(xi)]+inc(yi)]+    zi ];
    bab = p[p[p[inc(xi)]+    yi ]+inc(zi)];
    bbb = p[p[p[inc(xi)]+inc(yi)]+inc(zi)];

    double x1, x2, y1, y2;
    x1 = lerp(	grad (aaa, xf  , yf  , zf),				// The gradient function calculates the dot product between a pseudorandom
                  grad (baa, xf-1, yf  , zf),				// gradient vector and the vector from the input coordinate to the 8
                  u);										// surrounding points in its unit cube.
    x2 = lerp(	grad (aba, xf  , yf-1, zf),				// This is all then lerped together as a sort of weighted average based on the faded (u,v,w)
                  grad (bba, xf-1, yf-1, zf),				// values we made earlier.
                  u);
    y1 = lerp(x1, x2, v);

    x1 = lerp(	grad (aab, xf  , yf  , zf-1),
                  grad (bab, xf-1, yf  , zf-1),
                  u);
    x2 = lerp(	grad (abb, xf  , yf-1, zf-1),
                  grad (bbb, xf-1, yf-1, zf-1),
                  u);
    y2 = lerp (x1, x2, v);

    return (lerp (y1, y2, w)+1)/2;						// For convenience we bound it to 0 - 1 (theoretical min/max before is -1 - 1)
}

double Noise::grad(int hash, double x, double y, double z) {
    int h = hash & 15;									// Take the hashed value and take the first 4 bits of it (15 == 0b1111)
    double u = h < 8 /* 0b1000 */ ? x : y;				// If the most significant bit (MSB) of the hash is 0 then set u = x.  Otherwise y.

    double v;											// In Ken Perlin's original implementation this was another conditional operator (?:).  I
    // expanded it for readability.

    if(h < 4 /* 0b0100 */)								// If the first and second significant bits are 0 set v = y
        v = y;
    else if(h == 12 /* 0b1100 */ || h == 14 /* 0b1110*/)// If the first and second significant bits are 1 set v = x
        v = x;
    else 												// If the first and second significant bits are not equal (0/1, 1/0) set v = z
        v = z;

    return ((h&1) == 0 ? u : -u)+((h&2) == 0 ? v : -v);
}

double Noise::fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

int Noise::inc(int num) const {
    num++;
    if (repeat > 0) num %= repeat;
    return num;
}

double Noise::lerp(double a, double b, double x) {
    return a + x * (b - a);
}

unsigned int Noise::CreateTexture(int width, int height) {
    GLuint textureId;
    auto *data = new float[width * height];

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            data[y * width + x] = (float)GetNoise((float)x/(float)width, (float)y/(float)height, 1.0, 4, 2);

        }
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0,  GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    delete[] data;
    return textureId;


}
