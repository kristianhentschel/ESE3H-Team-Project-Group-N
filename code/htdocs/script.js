$(document).ready(function(){
	var data_update_timeout = 400;
	var previous_measurement = null;

	$(".screen").hide();
	$(".screen.main").show();
	
	$(".button.do-measure").click(function(){
			$(".results li").removeClass("invalid").removeClass("valid");
			$.get("/api/measure/", function(data) {
					//TODO handle "request ignored due to timeout" error response.
					console.log(data);
					setTimeout(data_update, data_update_timeout);
				});
		});

	$(".button.do-calibrate").click(function(){
			$(".results li").removeClass("invalid").removeClass("valid");
			$.get("/api/calibrate/", function(data){
					//TODO handle "request ignored due to timeout" error response.
					setTimeout(data_update, data_update_timeout);
					$(".screen").hide();
					$(".screen.main").show();
				});
		});

	$(".button.go-calibrate").click(function(){
			$(".screen").hide();
			$(".screen.calibrate").show();
		});
	
	$(".button.go-instructions").click(function(){
			$(".screen").hide();
			$(".screen.instructions").show();
		});

	$(".button.go-main").click(function(){
			$(".screen").hide();
			$(".screen.main").show();
		});


	function data_update() {
		$.getJSON("/api/data/", function(data){
				console.log(data);
				for (i = 1; i <= 4; i++) {
					$("#data-" + i).text(data.sensors[i].converted);
					$("#data-" + i + "-raw").text(data.sensors[i].value);
					if (previous_measurement == null) {
						if (data.sensors[i].time > 0) {
							$("#data-" + i).closest("li").addClass("valid");
						} else {
							$("#data-" + i).closest("li").addClass("invalid");
						}
					} else if (data.sensors[i].time == previous_measurement.sensors[i].time) {
						$("#data-" + i).closest("li").addClass("invalid");
					} else {
						$("#data-" + i).closest("li").addClass("valid");
					}
				}
				previous_measurement = data;
			});
	
	}
});

