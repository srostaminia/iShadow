 %% Line plot
% %error2modelMat=[1-annMeanErrorAllSub' 1-ciderMeanErrorAllSub' 1-lineMeanErrorAllSub'];
% %%std2modelMat=1.96*[annStdErrorAllSub' ciderStdErrorAllSub' lineStdErrorAllSub'];
% %ind2modelMat=[annIndPerAllSub ciderIndPerAllSub ciderIndPerAllSub];
% 
% lambdaList=[0.000100,0.000215,.000464,0.001000,0.002154,0.004642,0.010000,0.021544,0.046416,0.100000];
% %compare_barGraph(error2modelMat,subList,nLambda)
% 
% nUsers=length(subList);
% 
% figure;
% errorbar(lambdaList,annMeanErrorAllSub,annStdErrorAllSub);
% %errorbar(1:1:nLambda,annMeanErrorAllSub,annStdErrorAllSub);
% %errorbar((annIndPerAllSub),annMeanErrorAllSub,annStdErrorAllSub);
% 
% hold on;
% %errorbar(1:1:nLambda,ciderStdErrorAllSub,ciderStdErrorAllSub);
% errorbar(lambdaList,ciderStdErrorAllSub,ciderStdErrorAllSub);
% %errorbar((ciderIndPerAllSub),ciderStdErrorAllSub,ciderStdErrorAllSub);
% 
% hold off;
% 
% %errorb([(1:1:nLambda)' (1:1:nLambda)' (1:1:nLambda)'],error2modelMat);%,std2modelMat);
% % XL = get(gca,'ylim');
% % xlim([-2.2 XL(2)])
% %xlim([0 100])
% %xlim([0 nLambda+1])
% YL = get(gca,'ylim');
% set(gca,'ylim',[-0.1 YL(2)]) % Adjust only the lower limit.
% ylabel('Error (#pixels)');
% %xlabel('ANN Size(Percent Active Pixels)');
% xlabel('ANN Size(Regularization)');
% 
% 
% legend(findall(gca,'Tag','Box'),{'ANN error','CIDER error'},'Location','NorthEast');%,'LINE');  %%%%%CHANGE HERE!!
% suptitle('Error of pupil center prediction for ANN vs. CIDER');
% 
% plotFileName='pupil_avgAllUsers_error_lambda.pdf';
% 
% set(gca,'LineWidth',1.2);
% set(gcf, 'PaperPosition', [0  0 5 3.5]);  
% set(gcf, 'PaperSize', [5 3.5]); 
% saveas(gcf, plotFileName,'pdf')
% 
% 
% set(gca, ...
%   'Box'         , 'off'     , ...
%   'TickDir'     , 'out'     , ...
%   'TickLength'  , [.01 .01] , ...
%   'XMinorTick'  , 'off'      , ...
%   'YMinorTick'  , 'off'      , ...
%   'YGrid'       , 'off'      , ...
%   'XColor'      , [.3 .3 .3], ...
%   'YColor'      , [.3 .3 .3], ...
%   'LineWidth'   , 1.2         );
% 
% %  'YTick'       , 0:500:2500, ...
% 
