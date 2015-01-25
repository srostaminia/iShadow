clear;
close all;


pred=[6 4; 5 10; 1 2; 3 4];
true=[5 5];
diff=pred-repmat(true,[size(pred,1) 1])

figure;
scatter(0,0,100,'ob','filled');
hold on;
scatter(diff(:,1),diff(:,2),'or');
hold off;

legend('true pupil center','predicted pupil center');

halfDiff=1.25*max(abs(diff));
xlim([-1*halfDiff(1) halfDiff(1)]);
ylim([-1*halfDiff(2) halfDiff(2)]);


