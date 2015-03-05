function plotBar(var)

%var list-graphType,xVect,yVect,stdVect,ylimP,xlimP,ylabelP,xlabelP,titleP,pathToSave,plotFileName,saveOn,xticklist,legendlist
figure;

if strcmp(var.graphType,'bar')
    bar_custom(var.yVect,var.stdVect,var.xticklist,var.legendlist)
elseif strcmp(var.graphType,'line')
    errorbar(var.xVect,var.yVect,var.stdVect);
end

ylim(var.ylimP)
xlim(var.xlimP)
ylabel(var.ylabelP);
xlabel(var.xlabelP);
suptitle(var.titleP);
plotFormatQ_final(var.pathToSave,var.plotFileName,var.saveOn);
