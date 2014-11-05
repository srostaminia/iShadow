close all
imagesc(reshape(Out(1720,:), [111, 112]))
colormap('gray')
gca = imgca();
set(gca, 'XTick', []);
set(gca, 'YTick', []);

% hold on
% xunit = 10 * cos(th) + 55;
% yunit = 10 * sin(th) + 54;
% plot(xunit, yunit, 'LineWidth', 1.5)
% hold off

hold on
d = 10*2;
px = 55-10;
py = 54-10;
h = rectangle('Position',[px py d d],'Curvature',[1,1]);
daspect([1,1,1])
set(h, 'EdgeColor', [0, 0, 1])
set(h, 'LineWidth', 4)
hold off