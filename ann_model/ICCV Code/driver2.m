%Set octave graphics toolkit
graphics_toolkit('fltk');

%save matlab model to files
%save_model('results_init_strips_k7_lambda0.004642_rep1.mat')

%Load an eye image
load('eye_data2_manual_clean.mat');

%Load parameters
Wih = load('models/nn_wih.txt');
Who = load('models/nn_who.txt');
Bh = load('models/nn_bh.txt');
Bo = load('models/nn_bo.txt');
Mask = load('models/nn_mask.txt');

%load('results_init_strips_k7_lambda0.004642_rep1.mat')

%Set model size parameters
K = 6; %#hidden units
T = 2; %#targets
D = size(Mask,1); %#of pixels used

%Reconstruct parameter vector
W2ih=[];
for k=1:K
  tmp  = sparse(Mask(:,1),Mask(:,2),Wih(:,k),112,112);
  W2ih(:,k) = tmp(:);
end
W2ih = [W2ih;Bh];
W2ih = [zeros(112*112+1,1),W2ih];
W2ho = [Bo;Who];
W    = [W2ih(:);W2ho(:)];

%Display wights
for k=1:K
  Mimg1 = sparse(Mask(:,1),Mask(:,2),Wih(:,k),112,112);
  figure;
  imagesc(Mimg1);colormap gray;
end

%Loop through images and display
figure;
for i=1:size(X,1);
  [x,y] = nn_predict(img,Mask,Wih,Bh,Who,Bo,K,T,D);
  
  subplot(1,2,1);
  imagesc(reshape(X(i,1:end-1),[112,112]));
  colormap gray;

  subplot(1,2,2);
  imagesc(reshape(Out(i,1:end),[112,112]));
  colormap gray;
  hold on;
  plot(x,y,'r+','markersize',20);
  ginput(1);
end