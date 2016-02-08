$(function() {
	var progressbar_group = $("#progressbar-group");
	var progressbar = $("#progressbar");

	var failed_status_group = $("#failed-status-group");
	var error = $("#error");

	var success_status_group = $("#success-status-group");
	var link = $("#link");
	
	var input_file = $("#input-file");
	var input_filename = $("#input-filename");
	
	function hide_statuses() {
		failed_status_group.addClass("hidden");
		success_status_group.addClass("hidden");
	}
	
	function hide_progressbar() {
		progressbar_group.addClass("hidden");
	}
	
	function show_progressbar() {
		update_progressbar(0);
		progressbar_group.removeClass("hidden");
	}
	
	function update_progressbar(percent_value) {
		var percent_string = percent_value + "%";
		progressbar.width(percent_string);
		progressbar.text(percent_string);
	}

	function show_status(success, xhr) {
		console.log(xhr);
		if (success && !(/File upload error:/.test(xhr))) {
			link.html(xhr);
			success_status_group.removeClass("hidden");
		} else {
			if (!xhr)
				xhr = "something went wrong during the upload";
			error.text(xhr.replace(/File upload error:/,""));
			failed_status_group.removeClass("hidden");
		}
	}

	$("form").on("submit", function (e) {
		e.preventDefault();

		if (!input_file[0].files.length) {
			show_status(false, "Please select a file to upload");
			return;
		}

		var data = new FormData();
		data.append("file", input_file[0].files[0]);
		
		$.ajax({
			type: this.method,
			data: data,
			url: this.action,
			cache: false,
			contentType: false,
			processData: false,
			beforeSend: function() {
				hide_statuses();
				show_progressbar();
			},
			success: function(data){
				update_progressbar(100);
			},
			complete: function(xhr) {
				var success = xhr.readyState == XMLHttpRequest.DONE && xhr.status == 200;
				hide_progressbar();
				show_status(success, xhr.responseText);
			},
			uploadProgress: function(event, position, total, percentComplete) {
				update_progressbar(percentComplete);
			},
		});
	});

	input_file.on("change", function () {
		if (this.files.length)
			input_filename.val(this.files[0].name)
	});
});
