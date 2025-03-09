#include <sp2/graphics/gui/layout/grid.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

void GridLayout::update(P<Widget> container, Rect2d rect)
{
    Vector2i grid_size;
    int max_span = 1;
    for(P<Widget> w : container->getChildren())
    {
        if (!w || !w->isVisible())
            continue;
        Vector2i position = Vector2i(w->layout.position);
        Vector2i span = w->layout.span;
        grid_size.x = std::max(grid_size.x, position.x + span.x);
        grid_size.y = std::max(grid_size.y, position.y + span.y);
        max_span = std::max(max_span, span.x);
        max_span = std::max(max_span, span.y);
    }

    std::vector<float> col_width;
    col_width.resize(grid_size.x, 0.0f);
    std::vector<float> row_height;
    row_height.resize(grid_size.y, 0.0f);
    for(int n=0; n<grid_size.x; n++)
        col_width[n] = 0.0;
    for(int n=0; n<grid_size.y; n++)
        row_height[n] = 0.0;

    for(int span_size = 1; span_size <= max_span; span_size += 1)
    {
        for(P<Widget> w : container->getChildren())
        {
            if (!w || !w->isVisible())
                continue;
            Vector2i position = Vector2i(w->layout.position);
            Vector2i span = w->layout.span;

            if (span.x == span_size)
            {
                float widget_width = w->layout.size.x + w->layout.margin.left + w->layout.margin.right;
                float total_width = 0.0f;
                for(int x=position.x; x<position.x+span.x; x++)
                    total_width += col_width[x];
                float delta = widget_width - total_width;
                if (delta > 0.0f)
                {
                    for(int x=position.x; x<position.x+span.x; x++)
                    {
                        if (total_width == 0.0f)
                            col_width[x] = widget_width / float(span_size);
                        else
                            col_width[x] += delta * col_width[x] / total_width;
                    }
                }
            }
            if (span.y == span_size)
            {
                float widget_height = w->layout.size.y + w->layout.margin.top + w->layout.margin.bottom;
                float total_height = 0.0f;
                for(int y=position.y; y<position.y+span.y; y++)
                    total_height += row_height[y];
                float delta = widget_height - total_height;
                if (delta > 0.0f)
                {
                    for(int y=position.y; y<position.y+span.y; y++)
                    {
                        if (total_height == 0.0f)
                            row_height[y] = widget_height / float(span_size);
                        else
                            row_height[y] += delta * row_height[y] / total_height;
                    }
                }
            }
        }
    }

    std::vector<float> col_x;
    col_x.resize(grid_size.x, 0.0f);
    std::vector<float> row_y;
    row_y.resize(grid_size.y, 0.0f);
    for(int n=0; n<grid_size.x; n++)
    {
        col_x[n] = rect.position.x;
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
        auto old_position = w->layout.position;
        w->layout.position.x = w->layout.position.y = 0.0;
        cell_pos.y = rect.position.y + rect.size.y - cell_pos.y - cell_size.y;
        basicLayout(Rect2d(cell_pos, cell_size), *w);
        w->layout.position = old_position;
    }
}

}//namespace gui
}//namespace sp
