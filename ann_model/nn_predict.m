function [x,y] = nn_predict(img,Mask,Wih,Bh,Who,Bo,K,D);

%Compute Hidden Layer Activations
Ah = Bh; %Initialize hidden layer activations to hidden layer biases        
for i=1:D
  row = Mask(i,1); %row number 
  col = Mask(i,2); %column number
  % disp(img(row,col))
  x = img(row,col); %image value at (row,col)
  % disp(x)
  for k=1:K
    Ah(k) = Ah(k) + x*Wih(i,k); %Multiply image value by weight
  end
end

%Compute Hidden layer values and output
x=Bo(1); %Horizontal position
y=Bo(2); %Vertical position
for k=1:K
  h(k) = tanh(Ah(k)); %Apply non-linearity to activations
  x    = x + Who(k,1)*h(k); %accumulate x prediction
  y    = y + Who(k,2)*h(k); %accumulate y prediction
  % keyboard;
end
x=x*112;
y=y*111;