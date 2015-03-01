
clear;
x = [-pi:pi/20:pi];
y1 = sin(x);
y2 = cos(x);

figure
plot(x,y1,'-ro',x,y2,'-.b')
hTitle  = title ('My Publication-Quality Graphics');
hLegend=legend('sin(x)','cos(x)')
hXLabel = xlabel('Length (m)'                     );
hYLabel = ylabel('Mass (kg)'                      );

plotFileName='testPlot_final';

format_graph_paperQ(hTitle,hXLabel, hYLabel,hLegend,plotFileName)