#ifndef SP2_GRAPHICS_IMAGE_DRAW_H
#define SP2_GRAPHICS_IMAGE_DRAW_H

#include <sp2/graphics/image.h>


namespace sp::image {

#define PUT_PIXEL(_x, _y, _c) do { if ((_x) >= 0 && (_y) >= 0 && (_x) < size.x && (_y) < size.y) { pixels[(_x) + (_y) * size.x] = (_c); } } while(0)

template<typename T> void draw(ImageBase<T>& img, Ray2i ray, T color)
{
    auto size = img.getSize();
    auto pixels = img.getPtr();
    if (ray.end.x < ray.start.x)
        std::swap(ray.start, ray.end);
    int dx = ray.end.x - ray.start.x;
    int dy = std::abs(ray.end.y - ray.start.y);
    bool steep = dx < dy;
    if (steep)
    {
        std::swap(ray.start.x, ray.start.y);
        std::swap(ray.end.x, ray.end.y);
        std::swap(dx, dy);

        if (ray.end.x < ray.start.x)
            std::swap(ray.start, ray.end);
    }
    int sy = ray.start.y < ray.end.y ? 1 : -1;
    int err = 2*dy - dx;
    
    while(ray.start.x <= ray.end.x)
    {
        if (steep)
            PUT_PIXEL(ray.start.y, ray.start.x, color);
        else
            PUT_PIXEL(ray.start.x, ray.start.y, color);
        ray.start.x++;
        if (err > 0)
        {
            ray.start.y += sy;
            err -= 2*dx;
        }
        err += 2*dy;
    }
}

template<typename T> void draw(ImageBase<T>& img, Rect2i rect, T color)
{
    auto size = img.getSize();
    auto pixels = img.getPtr();

    for(int x=0; x<rect.size.x; x++)
    {
        PUT_PIXEL(rect.position.x + x, rect.position.y, color);
        PUT_PIXEL(rect.position.x + x, rect.position.y + rect.size.y - 1, color);
    }
    for(int y=1; y<rect.size.y; y++)
    {
        PUT_PIXEL(rect.position.x, rect.position.y + y, color);
        PUT_PIXEL(rect.position.x + rect.size.x - 1, rect.position.y + y, color);
    }
}

template<typename T> void draw(ImageBase<T>& target, Vector2i position, const ImageBase<T>& source)
{
    auto target_size = target.getSize();
    auto source_size = target.getSize();

    if (position.x + source_size.x <= 0 || position.x > target_size.x)
        return;
    if (position.y + source_size.y <= 0 || position.y > target_size.y)
        return;
    int w = std::min(source_size.x, target_size.x - position.x);
    const uint32_t* src = source.getPtr();
    uint32_t* dst = &target.getPtr()[position.x + position.y * target_size.x];
    if (position.x < 0)
    {
        src -= position.x;
        dst -= position.x;
        w += position.x;
    }
    int h = std::min(source_size.y, target_size.y - position.y);
    if (position.y < 0)
    {
        src -= position.y * source_size.x;
        dst -= position.y * target_size.x;
        h += position.y;
    }
    
    for(int y=0; y<h; y++)
    {
        memcpy(dst, src, w * sizeof(T));
        dst += target_size.x;
        src += source_size.x;
    }
}

template<typename T> void drawFilled(ImageBase<T>& img, Rect2i rect, T color)
{
    auto size = img.getSize();
    auto pixels = img.getPtr();

    for(int y=0; y<rect.size.y; y++)
        for(int x=0; x<rect.size.x; x++)
            PUT_PIXEL(rect.position.x + x, rect.position.y + y, color);
}

template<typename T> void drawCircle(ImageBase<T>& img, Vector2i position, int radius, T color)
{
    auto size = img.getSize();
    auto pixels = img.getPtr();

    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius / 2);

    while (x >= y)
    {
        PUT_PIXEL(position.x + x, position.y + y, color);
        PUT_PIXEL(position.x + y, position.y + x, color);
        PUT_PIXEL(position.x - y, position.y + x, color);
        PUT_PIXEL(position.x - x, position.y + y, color);
        PUT_PIXEL(position.x - x, position.y - y, color);
        PUT_PIXEL(position.x - y, position.y - x, color);
        PUT_PIXEL(position.x + y, position.y - x, color);
        PUT_PIXEL(position.x + x, position.y - y, color);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        
        if (err > 0)
        {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

template<typename T> void drawFilledCircle(ImageBase<T>& img, Vector2i position, int radius, T color)
{
    auto size = img.getSize();
    auto pixels = img.getPtr();

    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius / 2);

    while(x > 0)
    {
        for(int n=-x;n<=x; n++)
        {
            PUT_PIXEL(position.x + n, position.y + y, color);
            PUT_PIXEL(position.x + n, position.y - y, color);
        }

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        
        if (err > 0)
        {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

template<typename T> void drawFloodFill(ImageBase<T>& img, Vector2i position, T color)
{
    auto size = img.getSize();
    auto pixels = img.getPtr();

    if (position.x < 0 || position.y < 0 || position.x >= size.x || position.y >= size.y)
        return;
    auto target_color = pixels[position.x + position.y * size.x];
    if (target_color == color)
        return;
    std::vector<Vector2i> todo_list;
    todo_list.push_back(position);
    while(todo_list.size() > 0)
    {
        position = todo_list.back();
        todo_list.pop_back();
        
        if (pixels[position.x + position.y * size.x] != target_color)
            continue;
        pixels[position.x + position.y * size.x] = color;
        if (position.x > 0)
            todo_list.emplace_back(position.x - 1, position.y);
        if (position.y > 0)
            todo_list.emplace_back(position.x, position.y - 1);
        if (position.x < size.x - 1)
            todo_list.emplace_back(position.x + 1, position.y);
        if (position.y < size.y - 1)
            todo_list.emplace_back(position.x, position.y + 1);
    }
}

template<typename T> void drawFilledPolygon(ImageBase<T>& img, const std::vector<Vector2i>& polygon, T color)
{
    auto size = img.getSize();
    auto pixels = img.getPtr();

    if (polygon.size() < 3)
        return;
    int max = 0;
    int min = size.y;
    for(Vector2i p0 : polygon)
    {
        max = std::max(max, p0.y);
        min = std::min(min, p0.y);
    }
    for(int y=min; y<max; y++)
    {
        std::vector<int> nodes;
        Vector2i p0 = polygon.back();
        for(Vector2i p1 : polygon)
        {
            if ((p1.y < y && p0.y >= y) || (p0.y < y && p1.y >= y))
                nodes.push_back(p1.x + float(y-p1.y) / float(p0.y-p1.y) * float(p0.x-p1.x));
            p0 = p1;
        }
        
        std::sort(nodes.begin(), nodes.end());

        for(unsigned int n=0; n<nodes.size(); n+=2)
        {
            for(int x=nodes[n]; x<nodes[n+1]; x++)
                PUT_PIXEL(x, y, color);
        }
    }
}

#undef PUT_PIXEL

}//namespace sp


#endif//SP2_GRAPHICS_IMAGE_DRAW_H
