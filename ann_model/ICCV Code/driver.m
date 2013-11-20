%Set octave graphics toolkit
%graphics_toolkit('fltk');

%Load an eye image and out image
%Can be any images
load('eye_data2_manual_clean.mat');


%Load parameters
Wih = load('models/nn_wih.txt');
Who = load('models/nn_who.txt');
Bh = load('models/nn_bh.txt');
Bo = load('models/nn_bo.txt');
Mask = load('models/nn_mask.txt');

%Set model size parameters
K = 6; %#hidden units
T = 2; %#targets
D = size(Mask,1); %#of pixels used


for i=1:100
  img = reshape(X(i,1:end-1),[112,112]);
  out = reshape(Out(i,:),[112,112]);

  %Compute prediction
  [x(i),y(i)] = nn_predict(img,Mask,Wih,Bh,Who,Bo,K,T,D);
    
  %Display result
  subplot(1,2,1);
  imagesc(img);
  colormap gray;

  subplot(1,2,2);
  imagesc(out);
  colormap gray;
  hold on;
  plot(x,y,'r+','markersize',20);
end
[x(:),y(:)]