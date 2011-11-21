#ifndef UIGRAPHIC_H
#define UIGRAPHIC_H

struct Color
{
    int r, g, b;
};

struct LineStyle
{
    struct Color color;
    int lineWidth;
};

struct FillStyle
{
    struct Color color;
};

#endif /* UIGRAPHIC_H */
