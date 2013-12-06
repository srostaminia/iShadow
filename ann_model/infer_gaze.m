graphics_toolkit('fltk')
pkg load signal

data_path = '~/Desktop/eye_exper/dan_unadjusted'; %set path to directory of pngs
save_name = 'eye_data_dan_auto.mat';% name of mat file for output
display_skip = 1; %Set to >0 to display output for display_skip frames
data_name = 'dan'


res = [111,112];
instances = 100:7000;
%instances = 100:1000;
%X=ones(N,prod(res)+1);
j=0;

w=4;

if(0 & exist('template.mat'))
  load('template.mat')
else  
  template = zeros(2*w+1,2*w+1);
  template_instances = 1:2000:6000;
  % template_instances = 1:50:150;
  j = 1;
  for t=template_instances
    i=instances(t);
    name = sprintf('%s/%s_out_%06d.txt',data_path,data_name,i)
    img = double(load(name))/1000.0;
    img = img - quantile(img(:),0.05);
    img = img / quantile(img(:),0.99);
    img(img<0) = 0;
    img(img>1) = 1;

    
    figure(1);hold off;
    imagesc(img);colormap gray;hold on;
    [x,y] = ginput(1)
    x=round(x);
    y=round(y);
    if(x>w && x<res-w && y>w && y<res-w)
      figure(2);
      template(:,:,j) = double(img((y-w):(y+w),(x-w):(x+w)))-0.5;
      imagesc(template(:,:,j));
      colormap(gray);
      j=j+1;
    end  
  end
  save('template.mat','template');  
end

Out = zeros(length(instances),prod(res));
Eye = zeros(length(instances),prod(res));
gaze= zeros(length(instances),2);

j=1;
c=0;
xold = 1;
yold = 1;
img_out_old = zeros(res(1),res(2));
for i=instances
  name_out = sprintf('%s/%s_out_%06d.txt',data_path,data_name,i);
  name_eye = sprintf('%s/%s_eye_%06d.txt',data_path,data_name,i);
  num_match = 1;

  if(exist(name_out) && exist(name_eye))
    found_gaze = 0;
    img_eye = double(load(name_eye))/1000.0;
    img_out = double(load(name_out))/1000.0;
    
    img = img_out;
    img = img - quantile(img(:),0.05);
    img = img / quantile(img(:),0.99);
    img(img<0) = 0;
    img(img>1) = 1;
    
    
    xfound  = zeros(1,2);
    yfound  = zeros(1,2);
    quality = -inf*ones(1,2);
    
    %Create box for local search
    xrange = (xold-(2)*w):(xold+(2)*w);
    yrange = (yold-(2)*w):(yold+(2)*w);
    xrange = xrange(xrange>=1 & xrange <=res(1));
    yrange = yrange(yrange>=1 & yrange <=res(1));
    
    %Search over the whole image
    for t=1:size(template,3)
      XC = xcorr2(img-0.5,template(:,:,t),'coeff') ;
      maxXC = max(XC(:));
        
      if(maxXC>0.9)
        [y,x]=find(XC==maxXC);
        x=x-w;
        y=y-w;        
        if(length(x)==1)
          quality = maxXC;
          found_gaze = 1;   
          break;
        else
          xmid = (xrange(1)+xrange(end))/2;
          ymid = (yrange(1)+yrange(end))/2;
          score = (x-xmid).^2 + (y-ymid).^2;
          [foo,ind] = min(score);
          x=x(ind);
          y=y(ind);          
          quality = maxXC;
          found_gaze = 1; 
        end
       end   
     end   

    if(found_gaze==1)
      xold = x;
      yold = y;
      c=0;
    end
    
    %Try to determine if gaze target is outside of frame based on last 
    %known location. Wait max_skip steps for it to come back in.
    diff = sum(abs(img_out_old(:) - img_out(:)));   
    if(found_gaze ==0)
      if((xold<2*w || xold>res-2*w || yold<2*w || yold>res-2*w) && c<100)
        %if the frame has changed, update the skip counter
        c=c+1;
      elseif(xold>w && xold<res-w && yold>w && yold<res-w && c<2)  
        c=c+1;
      else
        %If more than max_skip steps have elapsed, get the gaze point from the user
        figure(1);imagesc(img_out);colormap gray;
        title(sprintf('%d  %.4f  %d %f',i,diff, c, maxXC));
        
        fprintf('Gaze point not found\n');
        % fprintf('Enter with mouse (yes) or skip (no)?>>');
        % cmd = kbhit()
        cmd = yes_or_no('Enter with mouse (yes) or skip (no)?>>');
        if (cmd==1) 
          fprintf('Click to enter gaze point>>')
          [x,y] = ginput(1); 
          x=round(x);
          y=round(y);
          xold = x;
          yold = y;
          if(x>w && x<res-w && y>w && y<res-w)
            %If the gaze point is valid, update it
            template(:,:,end+1) = double(img((y-w):(y+w),(x-w):(x+w)))-0.5;
            figure(2);
            imagesc(template(:,:,end));
            colormap(gray);
            found_gaze = 1;
          end
        end
        c=0;
      end    
    end
    
    %Display result of each step
    if(mod(j,display_skip)==0)
      figure(1);imagesc(img_out);colormap gray;
      title(sprintf('%d  %.4f  %d %f',i,diff, c, maxXC));
      hold on;
      plot([xrange(1),xrange(end),xrange(end),xrange(1), xrange(1)],[yrange(1),yrange(1),yrange(end),yrange(end),yrange(1)],'b-','linewidth',4);
      if(found_gaze==1)
        plot(x,y,'r+','markersize',20,'linewidth',4);
      end  
      hold off;
      drawnow;
    end

    if(found_gaze==1)
        Out(j,:) = img_out(:);
        Eye(j,:) = img_eye(:);
        gaze(j,:) = [x,y];
        j=j+1;
    end 
    j
    
  end
end

Eye = Eye(1:j-1,:);
Out = Out(1:j-1,:);

X=Eye*1000;
gout=gaze;
save('-V7',save_name,'Out','X','gout','template');
