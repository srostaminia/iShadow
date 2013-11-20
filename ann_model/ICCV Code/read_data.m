%data_path = '../data/addison1_adjusted'
%X=load([data_path, '/addison1.csv'])

data_path = '../data/addison2'
X=load([data_path, '/addison2.csv'])


Instances = X(:,1)';
g=X(:,2:3);
N=size(X,1);
res = [112,112]

X=ones(N,prod(res)+1);
j=1;
for i=Instances
  %img = imread(sprintf('%s/addison1_%06d_eye.png',data_path,i)) ;
  if(exist(sprintf('%s/addison2_%06d_eye.png',data_path,i)))
    img = imread(sprintf('%s/addison2_%06d_eye.png',data_path,i)) ;
    X(j,1:prod(res)) = img(:)';

    img = imread(sprintf('%s/addison2_%06d_out.png',data_path,i)) ;
    Out(j,1:prod(res)) = img(:)';

    figure(1);hold off;
    imagesc(img);colormap gray;hold on;
    plot((1-g(i+1,1)/600)*112,g(i+1,2)/600*112,'r+','markersize',20);
    title(sprintf('%d/%d',i,length(Instances)));
    [x,y] = ginput(1)
    gproj(j,:) = g(i+1,:);
    gout(j,:) = [x,y];
  end
  
  j=j+1;
end

%save('eye_data.mat','g','X');



save('eye_data2_manual.mat','gproj','gout','Out','X');

ind = find(gproj(:,2)==0);
gproj(ind,:)=[];
gout(ind,:)=[];
Out(ind,:)=[];
X(ind,:)=[];

save('eye_data2_manual_clean.mat','gproj','gout','Out','X');


for i=1:size(X,1)
    figure(1);hold off;
    imagesc(reshape(Out(i,:),[112,112]));colormap gray;hold on;
    plot(gout(i,1),gout(i,2),'r+','markersize',20);
    ginput(1);  
end
