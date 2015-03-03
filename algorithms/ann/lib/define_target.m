function y = define_target(g,spec,type)

  switch(type)
    case 'box'
      h = spec(1:2);
      v = spec(3:4);
      ind = h(1)<=g(:,1) & g(:,1)<=h(2) & v(1)<=g(:,2) & g(:,2)<=v(2);
    case 'radial'
      proto = spec(1:2);
      D     = sum(bsxfun(@minus,g,proto).^2,2);
      ind   = D<spec(3);
    case 'linear'
      D = g(:,1)*spec(1) + g(:,2)*spec(2) + spec(3);
      ind   = D<=0;
   end
   y   = 2*ind -1;