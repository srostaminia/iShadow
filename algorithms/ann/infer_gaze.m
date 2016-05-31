% graphics_toolkit('fltk')
% pkg load signal


data_file = '~/eye_exper/gaze_calib/gaze_calib.mat'; %set path to directory of pngs
save_name = 'addison_gaze.mat';% name of mat file for output
display_skip = 1; %Set to >0 to display output for display_skip frames
data_name = 'gaze_calib'

start_index = 100;
stop_index = 3700;

ask_reposition = false;
reuse_template = true;

res = [111,112];
instances = start_index:stop_index;
%instances = 100:1000;
%X=ones(N,prod(res)+1);
j=0;

w=4;

% seed = 136824521;

% rand('seed',seed); %set rand seed
% randn('seed',seed); %set randn seed

load(data_file);

figure(1);drawnow;imagesc(zeros(res(2)+50,res(1)));
figure(2);drawnow;

if(reuse_template && exist(sprintf('%s_template.mat', data_name)))
  load(sprintf('%s_template.mat', data_name))
else  
  template = zeros(2*w+1,2*w+1);
  template_instances = randperm(stop_index - start_index);
  j = 0;
  for t=template_instances
    i=instances(t);
    % name = sprintf('%s/%s_out_%06d.txt',data_path,data_name,i)
    % img = double(load(name))/1000.0;

    img = reshape(images_out(i,:), [111 112]);

    img = img - quantile(img(:),0.01);
    img = img / quantile(img(:),0.97);
    img(img<0) = 0;
    img(img>1) = 1;
    disp_img = [img; (zeros(50, 112) + 255)];
    
    figure(1);hold on;
    imshow(disp_img);colormap gray;axis on;hold off;
    [x,y] = ginput(1);

    x=round(x)
    y=round(y)
    if(x>w && x<res(2) && y>w && y<res(1))
      figure(2);

      if (x>res(2)-w)
        xrange = (res(2)-2*w):res(2);
      elseif (x<w)
        xrange = 0:2*w;
      else
        xrange = (x-w):(x+w);
      end

      if (y>res(1)-w)
        yrange = (res(1)-2*w):res(1);
      else
        yrange = (y-w):(y+w);
      end

      template(:,:,j+1) = double(img(yrange,xrange))-0.5;
      imagesc(template(:,:,j+1));
      colormap(gray);
      j=j+1

      if j >= 30
        break
      end
    end  
  end
  save(sprintf('%s_template.mat', data_name),'template');  
end

Out = zeros(length(instances),prod(res));
Eye = zeros(length(instances),prod(res));
gaze= zeros(length(instances),2);
gaze_label_idx= zeros(length(instances),1);

j=1;
c=0;
xold = 1;
yold = 1;
img_out_old = zeros(res(2),res(1));
jumped = 0;
for i=instances
  name_out = sprintf('%s_out_%06d.txt',data_name,i);
  name_eye = sprintf('%s_eye_%06d.txt',data_name,i);
  num_match = 1;

  % if(exist(name_out) && exist(name_eye))
    found_gaze = 0;
    % img_eye = double(load(name_eye))/1000.0;
    % img_out = double(load(name_out))/1000.0;

    img_eye = reshape(images_eye(i,:), [111 112]);
    img_out = reshape(images_out(i,:), [111 112]);

    fprintf('%s\n',name_out);
    
    img = img_out;
    img = img - quantile(img(:),0.01);
    img = img / quantile(img(:),0.97);
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
    
    best_template = 0;
    best_XC = 0;

    %Search over the whole image
    for t=1:size(template,3)
      % XC = xcorr2(img-0.5,template(:,:,t),'coeff') ;
      XC = normxcorr2(template(:,:,t), img-0.5) ;
      maxXC = max(XC(:));

      best_x = -1;
      best_y = -1;
        
      if(maxXC>0.70)
        [y,x]=find(XC==maxXC);
        x=x-w;
        y=y-w;

        if (maxXC > best_XC)
          best_XC = maxXC;
          best_template = t;
          best_x = x;
          best_y = y;
        end

        if(length(x)==1)
          quality = maxXC;
          found_gaze = 1;   
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

     if best_x >= 0
        x = best_x;
        y = best_y;
      end
        

    diff = sum(abs(img_out_old(:) - img_out(:)));   

    if(found_gaze==1)
      xdiff = abs(xold - x);
      ydiff = abs(yold - y);

      if( (((xdiff>3*w || ydiff>3*w))  || (jumped==1 && c>10)) && ask_reposition)
        figure(1);imshow(img);colormap gray;drawnow
        title(sprintf('%d  %.4f  %d %f',i,diff, c, maxXC));
        hold on;
        plot([xrange(1),xrange(end),xrange(end),xrange(1), xrange(1)],[yrange(1),yrange(1),yrange(end),yrange(end),yrange(1)],'b-','linewidth',4);
        if(found_gaze==1)
          plot(x,y,'r+','markersize',20,'linewidth',4);
        end  
        hold off;
        drawnow;

        % If the gaze point jumped, ask if it's invalid - if so, remove the template that caused it
        cmd = input('Gaze point jumped unexpectedly - is new position accurate (y / n)?','s');
        if (cmd == 'y')
          template(:,:,best_template)=[];
          save(sprintf('%s_template.mat', data_name),'template');  

          cmd2 = input('Enter with mouse (y) or skip (n)?>>','s');
          if (cmd2 == 'y')
            fprintf('Click to enter correct gaze point>>')
            [x,y] = ginput(1); 
            x=round(x);
            y=round(y);
            xold = x;
            yold = y;
            if(x>w && x<res(2)-w && y>w && y<res(1)-w)
              %If the gaze point is valid, update it
              template(:,:,end+1) = double(img((y-w):(y+w),(x-w):(x+w)))-0.5;
              figure(2);drawnow
              imagesc(template(:,:,end));
              colormap(gray);
              found_gaze = 1;
            end
            jumped = 0;
          else
            found_gaze = 0;
            jumped = 1;
            c = 0;
          end
        else
          jumped = 0;
        end
      end
      
      xold = x;
      yold = y;

      if (jumped==0)
        c=0;
      else
        c = c+1;
        found_gaze = 0;
      end
        
    end
    
    %Try to determine if gaze target is outside of frame based on last 
    %known location. Wait max_skip steps for it to come back in.
    if(found_gaze == 0 && jumped == 0)
      % if((xold<2*w || xold>res-2*w || yold<2*w || yold>res-2*w) && c<100)
      if((xold<2*w || xold>res(2)-2*w || yold<2*w || yold>res(1)-2*w) && c<200)
        %if the frame has changed, update the skip counter
        c=c+1;
      % elseif(xold>w && xold<res-w && yold>w && yold<res-w && c<25)  
      elseif(xold>w && xold<res(2)-w && yold>w && yold<res(1)-w && c<200)  
        c=c+1;
      else
        %If more than max_skip steps have elapsed, get the gaze point from the user
        figure(1);imshow(img);colormap gray;drawnow
        title(sprintf('%d  %.4f  %d %f',i,diff, c, maxXC));
        
        fprintf('Gaze point not found\n');
        % fprintf('Enter with mouse (yes) or skip (no)?>>');
        % cmd = kbhit()
        cmd = input('Enter with mouse (y) or skip (n)?>>','s');
        if (cmd=='y') 
          fprintf('Click to enter gaze point>>')
          [x,y] = ginput(1); 
          x=round(x);
          y=round(y);
          xold = x;
          yold = y;
          if(x>w && x<res(2)-w && y>w && y<res(1)-w)
            %If the gaze point is valid, update it
            template(:,:,end+1) = double(img((y-w):(y+w),(x-w):(x+w)))-0.5;
            figure(2);drawnow
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
      figure(1);imshow(img);colormap gray;drawnow
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
        gaze_label_idx(j) = i;
    end 
    j
    
  % end
end

Eye = Eye(1:j-1,:);
Out = Out(1:j-1,:);
gaze_label_idx = gaze_label_idx(1:j-1,:);
gaze = gaze(1:j-1,:);

X=Eye;
gout=gaze;
save('-V7',save_name,'Out','X','gout','gaze_label_idx','template');
