function compare_barGraph(matrix,subList)

    nUsers=length(subList);
    
    figure;
    bar(matrix);
    set(gca, 'XTick',1:nUsers, 'XTickLabel',subList)
    ylabel('% of annUsedFreq');
    suptitle('Error of pupil center prediction for ANN vs. CIDER');

end