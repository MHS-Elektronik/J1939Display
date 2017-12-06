/**
 * Gauge initialization
 *
 */
function refreshGaugeValue(setting, value) {
	var id = setting + 'Gauge';
	var gauge = document.getElementById(id);
	if ( gauge ) {
		if ( !value ) {
			var div = document.getElementById(setting);
			if ( div ) {
				value = div.innerHTML;
			}
		}
		if ( value ) {
			Gauge.Collection.get(id).setValue(value);
		}
	}
}

function generateGauges() {
 	var RpmGauge = new Gauge({
		renderTo    : 'RpmGauge',
		width       : 350,
		height      : 350,
		glow        : true,
		units       : 'x100',
		title       : "RPM",
		minValue    : 0,
		maxValue    : 3000,
		majorTicks  : ['0','5','10','15','20','25','30'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 5, "dec" : 0 },
		highlights  : [
      { from : 0, to : 400, color : 'rgba(255, 0, 0, .75)' },
      { from : 400, to : 600, color : 'rgba(255, 255, 0, .75)' },			
			{ from : 600, to : 2000, color : 'rgba(0, 255,  0, .75)' },
			{ from : 2000, to : 2500, color : 'rgba(255, 255, 0, .75)' },
			{ from : 2500, to : 3000, color : 'rgba(255, 0, 0, .75)' }
		],
	
		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});
	
	RpmGauge.draw();

	var EngineTorqueGauge = new Gauge({
		renderTo    : 'EngineTorqueGauge',
		width       : 350,
		height      : 350,
		glow        : true,
		units       : '%',
		title       : "Drehmoment",
		minValue    : 0,
		maxValue    : 100,
		majorTicks  : ['0','10','20','30','40','50','60','70','80','90','100'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 3, "dec" : 0 },

	
		highlights  : [
			{ from : 0,   to : 90, color : 'rgba(0, 255, 0, .75)' },
      { from : 90, to : 95, color : 'rgba(255, 255, 0, .75)' },
			{ from : 95, to : 100, color : 'rgba(255, 0, 0, .75)' }
			],
	
		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});

	EngineTorqueGauge.draw();

	var FuelConsumptionInLGauge = new Gauge({
		renderTo    : 'FuelConsumptionInLGauge',
		width       : 350,
		height      : 350,
		glow        : true,
		units       : 'l/h',
		title       : "Verbrauch",
		minValue    : 0,
		maxValue    : 100,
		majorTicks  : ['0','10','20','30','40','50','60','70','80','90', '100'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 3, "dec" : 1 },

	
		highlights  : [		
			{ from : 0, to : 100, color : 'rgba(0, 255,  0, .75)' }
		],
	
		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});

	FuelConsumptionInLGauge.draw();

	var WaterTempGauge = new Gauge({
		renderTo    : 'WaterTempGauge',
		width       : 350,
		height      : 350,
		glow        : true,
		units       : 'C',
		title       : "Wasser",
		minValue    : 40,
		maxValue    : 120,
		majorTicks  : ['40','50','60','70','80','90','100', '110', '120'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 3, "dec" : 0 },
		highlights  : [
			{ from : 40,   to : 100, color : 'rgba(0, 255,  0, .75)' },
			{ from : 100, to : 110, color : 'rgba(255, 255, 0, .75)' },
			{ from : 110, to : 120, color : 'rgba(255, 0, 0, .75)' }
		],

		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});

	WaterTempGauge.draw();

	
	var OilTempGauge = new Gauge({
		renderTo    : 'OilTempGauge',
		width       : 250,
		height      : 250,
		glow        : true,
		units       : 'C',
		title       : "Öltemperatur",
		minValue    : 40,
		maxValue    : 160,
		majorTicks  : ['40','60','80','100','120','140','160'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 2, "dec" : 0 },
		highlights  : [
			{ from : 40,   to : 110, color : 'rgba(0, 255,  0, .75)' },
			{ from : 110, to : 130, color : 'rgba(255, 255, 0, .75)' },    
			{ from : 130,   to : 160, color : 'rgba(255,  0,  0, .75)' }
		],

		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});
  
	OilTempGauge.draw();
  
	var OilLevelGauge = new Gauge({
		renderTo    : 'OilLevelGauge',
		width       : 250,
		height      : 250,
		glow        : true,
		units       : '%',
		title       : "Ölstand",
		minValue    : 0,
		maxValue    : 100,
		majorTicks  : ['0','10','20','30','40','50','60','70','80','90','100'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 3, "dec" : 0 },
    
		highlights  : [
			{ from : 0, to : 20, color : 'rgba(255, 0, 0, .75)' },
      { from : 20, to : 40, color : 'rgba(255, 255, 0, .75)' },			
			{ from : 40, to : 90, color : 'rgba(0, 255,  0, .75)' },
			{ from : 90, to : 95, color : 'rgba(255, 255, 0, .75)' },
			{ from : 95, to : 100, color : 'rgba(255, 0, 0, .75)' }
			],
	
		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});

	OilLevelGauge.draw();  
  
  
	var OilPressureGauge = new Gauge({
		renderTo    : 'OilPressureGauge',
		width       : 250,
		height      : 250,
		glow        : true,
		units       : 'Bar',
		title       : "Öldruck",
		minValue    : 0,
		maxValue    : 10,
		majorTicks  : ['0','1','2','3','4','5','6','7','8','9','10'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 2, "dec" : 1 },
		highlights  : [
      { from : 0.0, to : 0.3, color : 'rgba(255, 0, 0, .75)' },
      { from : 0.3, to : 0.5, color : 'rgba(255, 255, 0, .75)' },			
			{ from : 0.5, to : 9, color : 'rgba(0, 255,  0, .75)' },
			{ from : 9, to : 9.5, color : 'rgba(255, 255, 0, .75)' },
			{ from : 9.5, to : 10, color : 'rgba(255, 0, 0, .75)' }			
		],

		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});
    
	OilPressureGauge.draw();
  
	var FuelTempGauge = new Gauge({
		renderTo    : 'FuelTempGauge',
		width       : 250,
		height      : 250,
		glow        : true,
		units       : 'C',
		title       : "Kraftstoff",
		minValue    : 0,
		maxValue    : 120,
		majorTicks  : ['0','10','20','30','40','50','60','70','80','90','100','110','120'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 3, "dec" : 0 },
		highlights  : [
			{ from : 0, to : 60, color : 'rgba(0, 255,  0, .75)' },
			{ from : 60, to : 80, color : 'rgba(255, 255, 0, .75)' },
			{ from : 80, to : 120, color : 'rgba(255, 0, 0, .75)' }  
  		],

		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});
  
  FuelTempGauge.draw();
  
	var AirTempGauge = new Gauge({
		renderTo    : 'AirTempGauge',
		width       : 250,
		height      : 250,
		glow        : true,
		units       : 'C',
		title       : "Ladeluft",
		minValue    : 0,
		maxValue    : 120,
		majorTicks  : ['0','10','20','30','40','50','60','70','80','90','100','110','120'],
		minorTicks  : 2,
		strokeTicks : false,
		valueFormat      : { "int" : 3, "dec" : 0 },
		highlights  : [
			{ from : 0, to : 60, color : 'rgba(0, 255,  0, .75)' },
			{ from : 60, to : 80, color : 'rgba(255, 255, 0, .75)' },
			{ from : 80, to : 120, color : 'rgba(255, 0, 0, .75)' }  
  		],

		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});
  
  AirTempGauge.draw();  
  
  var TurbochargerPressureGauge = new Gauge({
		renderTo    : 'TurbochargerPressureGauge',
		width       : 250,
		height      : 250,
		glow        : true,
		units       : 'Bar',
		title       : "Ladedruck",
		minValue    : 0,
		maxValue    : 4,
		majorTicks  : ['0','1','2','3','4'],
		minorTicks  : 5,
		strokeTicks : false,
		valueFormat      : { "int" : 1, "dec" : 2 },
		highlights  : [
			{ from : 0,   to : 4, color : 'rgba(0, 255,  0, .75)' }
		],

		colors      : {
			plate      : '#222',
			majorTicks : '#f5f5f5',
			minorTicks : '#ddd',
			title      : '#fff',
			units      : '#ccc',
			numbers    : '#eee',
			needle     : { start : 'rgba(240, 128, 128, 1)', end : 'rgba(255, 160, 122, .9)' }
		}
	});
    
	TurbochargerPressureGauge.draw();
    
}