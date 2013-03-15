$(document).ready(function(){
	var data_update_manually = false;
	var data_update_timeout = 400;

	$(".screen").hide();
	$("#screen-main").show();
	
	$(".button.do-measure").click(function(){
			$(".results li").removeClass("valid");
			$.get("/api/measure/", function(data) {
					console.log(data);
					if (!data_update_manually) {
						setTimeout(data_update, data_update_timeout);
					}
				});
		});

	$(".button.do-calibrate").click(function(){
			$(".results li").removeClass("valid");
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
				for (i = 1; i <= 4; i++) {
					$("#data-" + i).text(data.sensors[i].converted);
					$("#data-" + i + "-raw").text(data.sensors[i].value);
				}
				$(".results li").addClass("valid");
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

