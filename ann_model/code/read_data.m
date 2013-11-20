data_path = '../data/addison1_adjusted'
X=load([data_path, '/addison1.csv'])
Instances = X(:,1)';
g=X(:,2:3);
N=size(X,1);
res = [112,112]

X=ones(N,prod(res)+1);
j=1;
for i=Instances
  img = imread(sprintf('%s/addison1_%06d_eye.png',data_path,i)) ;
  X(j,1:prod(res)) = img(:)';
  j=j+1;
end

save('eye_data.mat','g','X');