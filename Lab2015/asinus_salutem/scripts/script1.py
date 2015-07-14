#fctx1 = func_str_to_sym('A*cos(x)+B*sin(y)+[1^2*cos(x)]');
#print fctx1;
#print func_dist(fctx1['lambd_vars'], fctx1['lambd_f'], fctx1['lambd_f'], (-1,1,0.1));
#print func_dist(fctx1['lambd_vars'], fctx1['lambd_f'], fctx1['lambd_df'][0], (-1,1,0.1));
#print '----'
fctx_e2 = func_str_to_sym('( [ (C1*sin(x)-C2*cos(x))^2 + C3^2 ] - [ (C4*sin(y)-C5*cos(y))^2 + C6^2 ]  )^2')
#print fctx_e2
fctx_de2_th1 = func_str_to_sym('2*( [ (C1*sin(x)-C2*cos(x))^2 + C3^2 ] - [ (C4*sin(y)-C5*cos(y))^2 + C6^2 ]  )* (- (sqrt(C1^2+C3^2))^2 * sin(2*[x+atan2(C1,C2)] )  ) ')
#print fctx_de2_th1
print func_dist(fctx_e2['lambd_vars'], fctx_e2['lambd_df'][0], fctx_de2_th1['lambd_f'], (-1,1,0.1));
print func_dist(fctx_e2['lambd_vars'], fctx_e2['lambd_f'], fctx_de2_th1['lambd_f'], (-1,1,0.1));
