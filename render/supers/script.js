function start(world, raw_data)
{
  var scale_txt = $("#scaler");
  var GET = getUrlVars();
  var type = 'flops';
  var data = [];

  if(typeof raw_data == 'undefined')
    return $.get( "data.csv", function( data ) {
        start(world, $.csv.toObjects(data));
    });

  if(typeof GET['type'] != 'undefined')
    type = GET['type'];

  if(type == 'flops')
  {
    $("#watts").hide();
  }

  var col_name = 'Host name';
  var col_flops = 'Peak GFLOPs';
  var col_wattflop = 'Sus MFLOP/Watt';
  var col_comp = type; //comparsion column name

  ///extract the flops as number
  for(i = 0; i < raw_data.length; ++i)
  {
    if(raw_data[i][col_wattflop] == 0.0)
      raw_data[i][col_wattflop] = "-";

    raw_data[i]['flops'] = parseNumber(raw_data[i][col_flops]);
    raw_data[i]['wattflops'] = parseNumber(raw_data[i][col_wattflop]);

    ///note which is the first row to have all data, since we start at that row
    if(!isNaN(raw_data[i][type]) && raw_data[i][type] > 0)
      data.push(raw_data[i]);

    if(isNaN(raw_data[i]['wattflops'])) raw_data[i]['wattflops']=0.00;
    if(isNaN(raw_data[i]['flops'])) raw_data[i]['flops']=0.00;

    console.log('data calc: '+raw_data[i][col_name] + ' flops: ' + raw_data[i][col_flops] + ' ['+ parseNumber(raw_data[i]['flops']) + '] wattflops: '+raw_data[i]['wattflops']);
  }

  var data_time_scale = 10000;
  var highest_value = 0; ///largest value seen yet
  var last_scale = 1; ///last value for scaling
  var objs = [];
  var objradius = [];
  var start_time = 0;
  var colors = palette('tol-rainbow', data.length);
  colors.sort(function() {
      return .5 - Math.random();
  });

  // subscribe to ticker to advance the simulation
  Physics.util.ticker.on(function( time, dt ){
      world.step( time );

      if(start_time == 0)
        start_time = time;
      else
      {
        timestep = Math.round((time - start_time) / data_time_scale);
        if(timestep <= data.length)
        {
          for(i = 0; i < timestep; ++i)
          {
            //console.log('check obj='+i+ ' obj='+(i in objs));
            if(!(i in objs))
            {
              var default_radius = 15000;

              name=data[i][col_name];
              if(name == '' ||  name == '-') name=data[i]['Equipment'];
              value=data[i][col_comp];
              if(value == '' ||  value == '-') value='0.01';
              color=colors[i];

              var radius = default_radius;
              if(highest_value > value)
              {
                radius *= value / highest_value;
              }

              console.log('add name='+name+' Highest='+highest_value+' value='+value+' radius='+radius);

              /////var obj = Bodies.rectangle(world_width / 2, radius, radius, radius);
              var obj = null;
              for(is = 0; is < data.length; ++is)
                if(is in objs)
                {
                  is_obj = objs[is];
                  is_name=data[is][col_name];

                  //must be a resize event
                  if(is_name == name)
                  {
                    obj = is_obj;
                    obj.state.pos.set(250,150);
                    color = colors[is];
                    break;
                  }
                }
              if(obj == null)
              {
                obj = Physics.body('circle', {
                    x: 250,
                    y: 100,
                    vx: 0.01,
                    radius: radius,
                    styles: {
                      fillStyle: '#' + color,
                      lineWidth: 1,
                      strokeStyle: '#' + color,
                      angleIndicator: '#' + color
                    }
                });
                world.add( obj );
              }
              objs[i] = obj;
              objradius[i] = radius;
              scale_txt.text(name);
              var tdata = [];
              if(type == 'flops')
                tdata =[ data[i]['ls'],name,data[i]['Equipment'], data[i][col_flops] ]
              else
                tdata =[ data[i]['ls'],name,data[i]['Equipment'], data[i][col_flops], data[i]['Sus MFLOP/Watt']  ]

              var dup = 0;
              for(is = 0; is < data.length && is < i; ++is)
                if(data[is][col_name] == name)
                  ++dup;

              addtableentry(i, tdata, dup, '#' + color, '#' + color );
              world.add( obj );
              //console.log('add '+name +' at '+value);

              if(highest_value < value)
              {
                console.log('old Highest='+highest_value+' name='+name+' new highest='+value);
                highest_value = value;
              }

              ///re-scale existing for new size
              for(is = 0; is < data.length; ++is)
                if(is in objs)
                {
                  obj = objs[is];
                  name=data[is][col_name];

                  rvalue = data[is][col_comp]; //real value
                  nscale = rvalue / highest_value;// new scale (for largest)
                  //nvalue = nscale * default_radius;// new value
                  nvalue = Math.sqrt((nscale * default_radius) / Math.PI); //adjust for surface area

                  ///force every obj to wake up on scale change otherwise they get hung in air
                  obj.sleep(false);

                  console.log('rescale obj='+name+' rvalue='+rvalue+' nscale='+nscale+' nvalue='+nvalue);
                  obj.radius = obj.geometry.radius = nvalue;
                  obj.view = null;
                  obj.recalc();
                }
            }
          }
        }
      }
  });
}

var rows_visible = 0;
var last_row = 0;
function addtableentry(id, data, dup, fillcolor, bordercolor)
{
  if(rows_visible > 12)
  {
    $("#row"+(last_row - 1)).hide();
    last_row++;
    rows_visible--;
  }

  pstyle='style="border: 1px solid '+bordercolor+';white-space:nowrap;"';
  style='style="border: 1px solid '+bordercolor+';"';
  str = '<td style="border: 1px solid '+bordercolor+'; background-color:'+fillcolor+';"/>';
  for(i = 0; i < data.length; ++i)
  {
    str += '<td '+style+'>';

    //stop dups from having description twice
    if(dup != 0 && i == 2)
      data[i] = '<span '+pstyle+'>Update '+dup+'</span>';

    str += data[i];

    //if(dup != 0 && i == 1)
    //  str += '&nbsp;<span '+pstyle+'>Update '+dup+'</span>';

    str += '</td>';
  }

  if(last_row == 0) last_row = id;
  $("#dtable").prepend('<tr id="row' +id+ '"'+ style +'>' + str + '</tr>');
  rows_visible++; 
}

function parseNumber(str) {
    str = String(str);
    str = str.replace(/[^\d\.\-\ ]/g, '');
    buf = Number(str);
    if(buf < 0) buf = 0;
    return buf;
}

Physics(function(world){

    var viewWidth = 500;
    var viewHeight = 600;

    var renderer = Physics.renderer('canvas', {
        el: 'viewport',
        width: viewWidth,
        height: viewHeight,
        meta: false, // don't display meta data
    });
    
    // add the renderer
    world.add( renderer );

    // render on each step
    world.on('step', function(){
        world.render();
    });
    
    // bounds of the window
    var viewportBounds = Physics.aabb(0, 100, viewWidth, viewHeight);
    
    // constrain objects to these bounds
    world.add(Physics.behavior('edge-collision-detection', {
        aabb: viewportBounds,
        restitution: 0.75,
        cof: 0.99
    }));
    
    // ensure objects bounce when edge collision is detected
    world.add( Physics.behavior('body-impulse-response') );
    
    // add some gravity
    world.add( Physics.behavior('constant-acceleration', {
      acc: { x : 0, y: 0.0001 } 
    }) );
    world.add( Physics.behavior('constant-acceleration') );
    world.add( Physics.behavior('body-collision-detection') );
    world.add( Physics.behavior('sweep-prune') );
    
    // start the ticker
    Physics.util.ticker.start();

    start(world);
});

