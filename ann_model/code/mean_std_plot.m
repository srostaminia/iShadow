function h=mean_std_plot(x,y_mean,y_std,color,style,marker,axis_type)

if(size(y_std,2)==1)
  y_std = [y_std,-y_std];
end

if(strcmp(axis_type,'loglog'))
  h=loglog(x,y_mean,color,'LineWidth',5,'MarkerSize',10);hold on;
  loglog(x,y_mean(:) + (y_std(:,1)-y_mean(:)),[color,'--'],'LineWidth',1);
  loglog(x,y_mean(:) + (y_std(:,2)-y_mean(:)),[color,'--'],'LineWidth',1);
elseif(strcmp(axis_type,'plain'))
  h=plot(x,y_mean,color,'LineWidth',2);hold on;
  plot(x,y_mean(:)+y_std(:,1),color,'LineWidth',1);
  plot(x,y_mean(:)+y_std(:,2),color,'LineWidth',1);
elseif(strcmp(axis_type,'semilogy'))
  h=semilogy(x,y_mean,color,'LineWidth',2);hold on;
  semilogy(x,y_mean+y_std,color,'LineWidth',1);
  %semilogy(x,y_mean-y_std,color,'LineWidth',1);
elseif(strcmp(axis_type,'semilogx'))
  h=semilogx(x,y_mean,[color,marker,style],'LineWidth',3,'MarkerSize',8,'MarkerFaceColor',color);hold on;
  semilogx(x,y_mean(:)+y_std(:,1),color,'LineWidth',1);
  semilogx(x,y_mean(:)+y_std(:,2),color,'LineWidth',1);
else
  error('Unknown axis type');
end