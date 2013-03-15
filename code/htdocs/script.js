$(document).ready(function(){
	var data_update_manually = false;
	var data_update_timeout = 400;

	$(".screen").hide();
	$("#screen-main").show();
	
	$(".button.do-measure").click(function(){
			$.get("/api/measure/", function(data) {
					console.log(data);
					if (!data_update_manually) {
						setTimeout(data_update, data_update_timeout);
					}
				});
		});

	$(".button.do-calibrate").click(function(){
			$.get("/api/calibrate/", function(data){
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
				
				$("#data-1-raw").text(data.sensors[1].value);
				$("#data-2-raw").text(data.sensors[2].value);
				$("#data-3-raw").text(data.sensors[3].value);
				$("#data-4-raw").text(data.sensors[4].value);
			});
	
	}


	if (data_update_manually) {
		$(".button.do-data").click(function(){
			});
	} else {
		$(".do-data").hide();
		data_update();
	}
});

