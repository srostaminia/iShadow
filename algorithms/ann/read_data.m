clear all; close all;
graphics_toolkit('fltk')

%data_path = '../data/addison2_adjusted'
%X=load([data_path, '/addison2.csv'])

data_path = '/Users/ammayber/Desktop/eye_exper/addison2_unadjusted'

% Number of random samples to choose
N=100;

% Index to start choosing from
start_N = 100;

% Index to stop choosing at
stop_N = 6397;

Instances=randperm(stop_N - start_N) + start_N;

res = [111,112];

X=ones(N,prod(res)+1);
j=1;
for i=Instances
  %img = imread(sprintf('%s/addison2_%06d_eye.png',data_path,i)) ;
  if(exist(sprintf('%s/addison2_eye_%06d.png',data_path,i)))
    img_txt = double(load(sprintf('%s/addison2_eye_%06d.txt',data_path,i))) ;
    X(j,1:prod(res)) = img_txt(:)';

    img_txt = double(load(sprintf('%s/addison2_out_%06d.txt',data_path,i))) ;
    Out(j,1:prod(res)) = img_txt(:)';

    disp_img = double(imread(sprintf('%s/addison2_out_%06d.png',data_path,i)));
    disp_img = [disp_img; (zeros(50, 112) + 255)];
    figure(1);hold off;
    imagesc(disp_img);colormap gray;hold on;
    set(gca,'dataAspectRatio',[1 1 1]);
    title(sprintf('Image %06d (%d/%d)',i,j,N));

    while true
      [x,y] = ginput(1)
      % close all;

      % ans = yes_or_no('Dot visible?');

      if (y <= 112)
        gout(j,:) = [x,y];
        j=j+1;
        break;
      elseif (y < 125)
        disp('Unsure whether that is in the valid range or not, please try again');
      else
        break;
      end
    end

    if j > N
      break;
    end
  end
end

%save('eye_data.mat','g','X');

save('addison2_data.mat','gout','X','Out');


for i=1:size(X,1)
    figure(1);hold off;
    imagesc(reshape(Out(i,:),[111,112]));colormap gray;hold on;
    plot(gout(i,1),gout(i,2),'r+','markersize',20);
    ginput(1);  
end
