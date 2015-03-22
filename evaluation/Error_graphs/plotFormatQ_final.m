function plotFormatQ_final(pathToSave,plotFileName,saveOn,plotMode)

set(gca,'LineWidth',3);

if strcmp(plotMode,'subplot')
    a=[20 8];
else
    a=[6 4];
end
    
set(gcf, 'PaperPosition', [ [0  0], a]);  
set(gcf, 'PaperSize', a); 


set(gca, ...
  'Box'         , 'off'     , ...
  'TickDir'     , 'out'     , ...
  'TickLength'  , [.01 .01] , ...
  'XMinorTick'  , 'off'      , ...
  'YMinorTick'  , 'off'      , ...
  'YGrid'       , 'on'      , ...
  'XColor'      , [.3 .3 .3], ...
  'YColor'      , [.3 .3 .3], ...
  'LineWidth'   , 2        );

if saveOn
    origin=pwd;
    
    cd ~
    cd(pathToSave);
    
    saveas(gcf, plotFileName,'png')
    cd (origin);

end


