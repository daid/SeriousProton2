#include <sp2/graphics/gui/layout/grid.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

SP_REGISTER_LAYOUT("grid", GridLayout);

void GridLayout::update(P<Widget> container, Rect2d rect)
{
    Vector2i grid_size;
    for(P<Widget> w : container->getChildren())
    {
        if (!w || !w->isVisible())
            continue;
        Vector2i position = Vector2i(w->layout.position);
        Vector2i span = w->layout.span;
        grid_size.x = std::max(grid_size.x, position.x + span.x);
        grid_size.y = std::max(grid_size.y, position.y + span.y);
    }
    
    float col_width[grid_size.x];
    float row_height[grid_size.y];
    for(int n=0; n<grid_size.x; n++)
        col_width[n] = 0.0;
    for(int n=0; n<grid_size.y; n++)
        row_height[n] = 0.0;

    for(P<Widget> w : container->getChildren())
    {
        if (!w || !w->isVisible())
            continue;
        Vector2i position = Vector2i(w->layout.position);
        Vector2i span = w->layout.span;
        
        if (span.x == 1)
        {
            float width = w->layout.size.x + w->layout.margin.left + w->layout.margin.right;
            col_width[position.x] = std::max(col_width[position.x], width);
        }
        if (span.y == 1)
        {
            float height = w->layout.size.y + w->layout.margin.top + w->layout.margin.bottom;
            row_height[position.y] = std::max(row_height[position.y], height);
        }
    }
    //SP2TODO: Handle the case where span > 1
    
    float col_x[grid_size.x];
    float row_y[grid_size.y];
    for(int n=0; n<grid_size.x; n++)
    {
        col_x[n] = 0.0;
        for(int m=0; m<n; m++)
            col_x[n] += col_width[m];
    }
    for(int n=0; n<grid_size.y; n++)
    {
        row_y[n] = 0.0;
        for(int m=0; m<n; m++)
            row_y[n] += row_height[m];
    }
    
    for(P<Widget> w : container->getChildren())
    {
        if (!w || !w->isVisible())
            continue;
        Vector2i position = Vector2i(w->layout.position);
        Vector2i span = w->layout.span;

        Vector2d cell_pos(col_x[position.x], row_y[position.y]);
        Vector2d cell_size(col_width[position.x], row_height[position.y]);
        for(int n=1; n<span.x; n++)
            cell_size.x += col_width[position.x + n];
        for(int n=1; n<span.y; n++)
            cell_size.y += row_height[position.y + n];
        basicLayout(Rect2d(cell_pos, cell_size), *w);
    }
}

}//namespace gui
}//namespace sp
